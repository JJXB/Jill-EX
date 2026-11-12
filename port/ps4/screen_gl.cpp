// PS4 display backend: OpenGL ES 2.0 through Piglet/EGL.
//
// The game still renders into the 8-bit 320x200 Framebuffer; this file only
// uploads that frame as an RGBA texture and draws one textured quad, scaled to
// the largest integer fit inside the 1920x1080 output.
//
// Shaders come from the firmware's libScePrecompiledShaders module (PSSL
// binaries loaded with glShaderBinary).  Piglet has no GLSL compiler, so do not
// replace these with glShaderSource without a PSSL shader built elsewhere.

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <orbis/Pigletv2VSH.h>
#include <orbis/Sysmodule.h>
#include <orbis/SystemService.h>
#include <orbis/libkernel.h>

#include "../src/platform_paths.h"
#include "../src/screen.h"

// Blob layout used by libScePrecompiledShaders (see the OpenOrbis piglet
// sample).  Declared at global scope so the symbol keeps external linkage.
struct OrbisShaderBlob {
    char* ident;
    unsigned char hash[16];
    uint64_t len;
    unsigned char* code;
};
extern "C" OrbisShaderBlob scePrecompiledShaderEntries[];

namespace {

constexpr int kOutputWidth = 1920;
constexpr int kOutputHeight = 1080;

using ShaderBlob = std::pair<unsigned char*, uint64_t>;

struct Mat4 {
    float m[16];
};

Mat4 mat_identity() {
    Mat4 r{};
    r.m[0] = 1.0f;
    r.m[5] = 1.0f;
    r.m[10] = 1.0f;
    r.m[15] = 1.0f;
    return r;
}

// Orthographic projection with y pointing down (0 at the top edge).
Mat4 mat_ortho_y_down(float w, float h) {
    Mat4 r = mat_identity();
    r.m[0] = 2.0f / w;
    r.m[5] = -2.0f / h;
    r.m[12] = -1.0f;
    r.m[13] = 1.0f;
    return r;
}

// Translate(x, y) * Scale(sx, sy) for a unit quad.
Mat4 mat_quad(float x, float y, float sx, float sy) {
    Mat4 r = mat_identity();
    r.m[0] = sx;
    r.m[5] = sy;
    r.m[12] = x;
    r.m[13] = y;
    return r;
}

class ScreenGL final : public Screen {
public:
    ~ScreenGL() override { shutdown(); }

    bool init(int pixel_w, int pixel_h, int mode, std::string& err) override {
        (void)pixel_w;
        (void)pixel_h;
        (void)mode;
        jill_paths::trace("gl: init");
        if (!load_modules(err)) {
            return false;
        }
        if (!create_context(err)) {
            return false;
        }
        if (!create_program(err)) {
            return false;
        }
        // Match the OpenOrbis sample order: the system splash is hidden after
        // the Piglet context exists, not before it.
        sceSysmoduleLoadModuleInternal(ORBIS_SYSMODULE_INTERNAL_SYSTEM_SERVICE);
        jill_paths::trace("gl: hiding system splash");
        sceSystemServiceHideSplashScreen();
        jill_paths::trace("gl: system ready");
        jill_paths::trace("gl: ready");
        return true;
    }

    void apply(int pixel_w, int pixel_h, int mode, int display_w,
               int display_h) override {
        // The output is always 1080p; the quad fit is recomputed per frame.
        (void)pixel_w;
        (void)pixel_h;
        (void)mode;
        (void)display_w;
        (void)display_h;
    }

    void present(const uint32_t* argb, int w, int h) override {
        if (!context_ready_ || w <= 0 || h <= 0) {
            return;
        }
        if (!first_frame_) {
            first_frame_ = true;
            jill_paths::trace("gl: first present %dx%d", w, h);
        }
        ensure_texture(w, h);
        if (!texture_) {
            return;
        }

        // 0xAARRGGBB (little-endian) -> RGBA byte order for GLES2.
        rgba_.resize(static_cast<size_t>(w) * h * 4);
        for (size_t i = 0; i < static_cast<size_t>(w) * h; ++i) {
            const uint32_t c = argb[i];
            rgba_[i * 4 + 0] = static_cast<uint8_t>((c >> 16) & 0xFF);
            rgba_[i * 4 + 1] = static_cast<uint8_t>((c >> 8) & 0xFF);
            rgba_[i * 4 + 2] = static_cast<uint8_t>(c & 0xFF);
            rgba_[i * 4 + 3] = static_cast<uint8_t>((c >> 24) & 0xFF);
        }

        // Integer fit with letterbox/pillarbox.  A non-integer fallback only
        // runs on displays smaller than the source.
        float scale = std::min(static_cast<float>(kOutputWidth) / w,
                               static_cast<float>(kOutputHeight) / h);
        if (scale >= 1.0f) {
            scale = static_cast<float>(static_cast<int>(scale));
        }
        const float out_w = static_cast<float>(w) * scale;
        const float out_h = static_cast<float>(h) * scale;
        const float out_x = (kOutputWidth - out_w) * 0.5f;
        const float out_y = (kOutputHeight - out_h) * 0.5f;

        glBindTexture(GL_TEXTURE_2D, texture_);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA,
                        GL_UNSIGNED_BYTE, rgba_.data());

        glViewport(0, 0, kOutputWidth, kOutputHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glUseProgram(program_);

        const Mat4 projection = mat_ortho_y_down(kOutputWidth, kOutputHeight);
        const Mat4 model = mat_quad(out_x, out_y, out_w, out_h);
        const Mat4 texture_space = mat_identity();
        glUniformMatrix4fv(u_projection_, 1, GL_FALSE, projection.m);
        glUniformMatrix4fv(u_model_, 1, GL_FALSE, model.m);
        glUniformMatrix4fv(u_texture_space_, 1, GL_FALSE, texture_space.m);
        glUniform1f(u_opacity_, 1.0f);
        glUniform1i(u_sampler_, 0);

        static const GLfloat kQuad[] = {
            // x    y    u    v
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 0.0f,
        };
        glVertexAttribPointer(a_vertex_, 4, GL_FLOAT, GL_FALSE,
                              4 * sizeof(GLfloat), kQuad);
        glEnableVertexAttribArray(a_vertex_);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        if (!first_frame_error_checked_) {
            first_frame_error_checked_ = true;
            const GLenum gl_err = glGetError();
            if (gl_err != GL_NO_ERROR) {
                jill_paths::trace("gl: first draw error 0x%x", gl_err);
            } else {
                jill_paths::trace("gl: first draw ok (rect %.0f,%.0f %.0fx%.0f)",
                                  out_x, out_y, out_w, out_h);
            }
        }

        if (!eglSwapBuffers(egl_display_, egl_surface_)) {
            std::fprintf(stderr, "eglSwapBuffers failed: 0x%x\n", eglGetError());
        }
    }

    void set_title(const char* title) override {
        (void)title;  // no window title on the console
    }

    void shutdown() override {
        // GL objects must be destroyed while the context is still current.
        if (texture_) {
            glDeleteTextures(1, &texture_);
            texture_ = 0;
        }
        if (program_) {
            glDeleteProgram(program_);
            program_ = 0;
        }
        if (egl_display_ != EGL_NO_DISPLAY) {
            eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE,
                           EGL_NO_CONTEXT);
            if (egl_surface_ != EGL_NO_SURFACE) {
                eglDestroySurface(egl_display_, egl_surface_);
                egl_surface_ = EGL_NO_SURFACE;
            }
            if (egl_context_ != EGL_NO_CONTEXT) {
                eglDestroyContext(egl_display_, egl_context_);
                egl_context_ = EGL_NO_CONTEXT;
            }
            eglTerminate(egl_display_);
            egl_display_ = EGL_NO_DISPLAY;
        }
        context_ready_ = false;
        unload_modules();
    }

private:
    bool load_modules(std::string& err) {
        // Piglet and the precompiled shader table are firmware modules.  The
        // sandbox random word is the prefix used by the OpenOrbis samples.
        std::string prefix = sceKernelGetFsSandboxRandomWord();
        prefix = "/" + prefix + "/common/lib/";
        jill_paths::trace("gl: loading modules from %s", prefix.c_str());

        int mstart = 0;
        piglet_module_ = sceKernelLoadStartModule(
            (prefix + "libScePigletv2VSH.sprx").c_str(), 0, nullptr, 0, nullptr,
            &mstart);
        if (piglet_module_ < 0) {
            err = "failed to load libScePigletv2VSH.sprx (0x" +
                  std::to_string(piglet_module_) + ")";
            return false;
        }
        jill_paths::trace("gl: piglet module loaded (%d)", piglet_module_);
        shaders_module_ = sceKernelLoadStartModule(
            (prefix + "libScePrecompiledShaders.sprx").c_str(), 0, nullptr, 0,
            nullptr, &mstart);
        if (shaders_module_ < 0) {
            err = "failed to load libScePrecompiledShaders.sprx (0x" +
                  std::to_string(shaders_module_) + ")";
            return false;
        }
        jill_paths::trace("gl: shader module loaded (%d)", shaders_module_);
        return true;
    }

    void unload_modules() {
        if (shaders_module_ >= 0) {
            int mstop = 0;
            sceKernelStopUnloadModule(shaders_module_, 0, nullptr, 0, nullptr,
                                      &mstop);
            shaders_module_ = -1;
        }
        if (piglet_module_ >= 0) {
            int mstop = 0;
            sceKernelStopUnloadModule(piglet_module_, 0, nullptr, 0, nullptr,
                                      &mstop);
            piglet_module_ = -1;
        }
    }

    bool create_context(std::string& err) {
        OrbisPglConfig config{};
        config.size = sizeof(config);
        config.flags = ORBIS_PGL_FLAGS_USE_COMPOSITE_EXT |
                       ORBIS_PGL_FLAGS_USE_FLEXIBLE_MEMORY | 0x60;
        config.processOrder = 1;
        // These match the values in the OpenOrbis piglet sample: known to boot
        // on real hardware.  They are oversized for a 320x200 game; tune down
        // only after the first on-device run (see PS4_PORT_PLAN.md).
        config.systemSharedMemorySize = 250ull * 1024 * 1024;
        config.videoSharedMemorySize = 512ull * 1024 * 1024;
        config.maxMappedFlexibleMemory = 170ull * 1024 * 1024;
        config.drawCommandBufferSize = 1 * 1024 * 1024;
        config.lcueResourceBufferSize = 1 * 1024 * 1024;
        config.dbgPosCmd_0x40 = kOutputWidth;
        config.dbgPosCmd_0x44 = kOutputHeight;
        config.dbgPosCmd_0x48 = 0;
        config.dbgPosCmd_0x4C = 0;
        config.unk_0x5C = 2;

        if (!scePigletSetConfigurationVSH(&config)) {
            err = "scePigletSetConfigurationVSH failed";
            return false;
        }
        jill_paths::trace("gl: piglet configured");

        egl_display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (egl_display_ == EGL_NO_DISPLAY) {
            const EGLint egl_err = eglGetError();
            err = "eglGetDisplay failed (0x" + std::to_string(egl_err) + ")";
            return false;
        }
        EGLint major = 0;
        EGLint minor = 0;
        if (!eglInitialize(egl_display_, &major, &minor)) {
            err = "eglInitialize failed";
            return false;
        }
        jill_paths::trace("gl: eglInitialize %d.%d", major, minor);
        if (!eglBindAPI(EGL_OPENGL_ES_API)) {
            err = "eglBindAPI failed";
            return false;
        }
        eglSwapInterval(egl_display_, 0);
        jill_paths::trace("gl: eglBindAPI ok");

        const EGLint attribs[] = {
            EGL_RED_SIZE,     8,
            EGL_GREEN_SIZE,   8,
            EGL_BLUE_SIZE,    8,
            EGL_ALPHA_SIZE,   8,
            EGL_DEPTH_SIZE,   0,
            EGL_STENCIL_SIZE, 0,
            EGL_SAMPLE_BUFFERS, 0,
            EGL_SAMPLES,      0,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE,
        };
        EGLConfig egl_config = nullptr;
        EGLint num_configs = 0;
        if (!eglChooseConfig(egl_display_, attribs, &egl_config, 1,
                             &num_configs) ||
            num_configs < 1) {
            err = "eglChooseConfig found no config";
            return false;
        }
        jill_paths::trace("gl: eglChooseConfig ok");

        OrbisPglWindow render_window{
            0, static_cast<khronos_uint32_t>(kOutputWidth),
            static_cast<khronos_uint32_t>(kOutputHeight)};
        const EGLint window_attribs[] = {
            EGL_RENDER_BUFFER, EGL_BACK_BUFFER, EGL_NONE,
        };
        egl_surface_ = eglCreateWindowSurface(egl_display_, egl_config,
                                              &render_window, window_attribs);
        if (egl_surface_ == EGL_NO_SURFACE) {
            err = "eglCreateWindowSurface failed";
            return false;
        }
        jill_paths::trace("gl: window surface ok");

        const EGLint ctx_attribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE,
        };
        egl_context_ = eglCreateContext(egl_display_, egl_config, EGL_NO_CONTEXT,
                                        ctx_attribs);
        if (egl_context_ == EGL_NO_CONTEXT) {
            err = "eglCreateContext failed";
            return false;
        }
        if (!eglMakeCurrent(egl_display_, egl_surface_, egl_surface_,
                            egl_context_)) {
            err = "eglMakeCurrent failed";
            return false;
        }
        jill_paths::trace("gl: context current");
        jill_paths::trace("GL_VERSION: %s",
                          reinterpret_cast<const char*>(glGetString(GL_VERSION)));
        jill_paths::trace("GL_RENDERER: %s",
                          reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
        context_ready_ = true;
        return true;
    }

    ShaderBlob find_shader(const char* name) {
        for (OrbisShaderBlob* entry = scePrecompiledShaderEntries;
             entry->ident != nullptr; ++entry) {
            if (std::strcmp(name, entry->ident) == 0) {
                return {entry->code, entry->len};
            }
        }
        return {nullptr, 0};
    }

    bool create_program(std::string& err) {
        jill_paths::trace("gl: creating program");
        const ShaderBlob vert = find_shader("texmap/v_2.vert");
        const ShaderBlob frag = find_shader("texmap/f_2.frag");
        if (!vert.first || !frag.first) {
            err = "precompiled texmap shaders are not available";
            return false;
        }

        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderBinary(1, &vs, 0, vert.first, static_cast<GLsizei>(vert.second));
        glShaderBinary(1, &fs, 0, frag.first, static_cast<GLsizei>(frag.second));
        program_ = glCreateProgram();
        glAttachShader(program_, vs);
        glAttachShader(program_, fs);
        glLinkProgram(program_);
        glDeleteShader(vs);
        glDeleteShader(fs);

        u_model_ = glGetUniformLocation(program_, "u_modelViewMatrix");
        u_projection_ = glGetUniformLocation(program_, "u_projectionMatrix");
        u_texture_space_ = glGetUniformLocation(program_, "u_textureSpaceMatrix");
        u_sampler_ = glGetUniformLocation(program_, "s_sampler");
        u_opacity_ = glGetUniformLocation(program_, "u_opacity");
        a_vertex_ = glGetAttribLocation(program_, "a_vertex");
        if (u_model_ < 0 || u_projection_ < 0 || u_texture_space_ < 0 ||
            u_sampler_ < 0 || u_opacity_ < 0 || a_vertex_ < 0) {
            err = "texmap shader is missing an expected uniform/attribute";
            return false;
        }
        jill_paths::trace("gl: program linked");
        return true;
    }

    void ensure_texture(int w, int h) {
        if (texture_ && tex_w_ == w && tex_h_ == h) {
            return;
        }
        if (texture_) {
            glDeleteTextures(1, &texture_);
            texture_ = 0;
        }
        glGenTextures(1, &texture_);
        glBindTexture(GL_TEXTURE_2D, texture_);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        tex_w_ = w;
        tex_h_ = h;
    }

    int32_t piglet_module_ = -1;
    int32_t shaders_module_ = -1;
    EGLDisplay egl_display_ = EGL_NO_DISPLAY;
    EGLSurface egl_surface_ = EGL_NO_SURFACE;
    EGLContext egl_context_ = EGL_NO_CONTEXT;
    GLuint program_ = 0;
    GLuint texture_ = 0;
    GLint u_model_ = -1;
    GLint u_projection_ = -1;
    GLint u_texture_space_ = -1;
    GLint u_sampler_ = -1;
    GLint u_opacity_ = -1;
    GLint a_vertex_ = -1;
    int tex_w_ = 0;
    int tex_h_ = 0;
    bool context_ready_ = false;
    bool first_frame_ = false;
    bool first_frame_error_checked_ = false;
    std::vector<uint8_t> rgba_;
};

}  // namespace

std::unique_ptr<Screen> create_screen_gl() {
    return std::unique_ptr<Screen>(new ScreenGL());
}
