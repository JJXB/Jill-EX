# Regenerates the LiveArea artwork as 8-bit indexed PNGs.
#
# SceShell rejects VPKs whose LiveArea images are 24/32-bit, with
# 0x8010113D at the end of the install (VitaShell also reports
# 0xFFFFFFFF earlier).  Every image must be:
#
#   icon0.png     128x128   8-bit indexed   < 128 KiB
#   startup.png   280x158   8-bit indexed   <  64 KiB
#   bg.png        840x500   8-bit indexed   < 128 KiB
#   none of them interlaced
#
# System.Drawing always writes 32-bit RGBA, so the quantisation here goes
# through ffmpeg's palettegen/paletteuse (pal8).
#
# Usage:  powershell -ExecutionPolicy Bypass -File port\vita\make_livearea.ps1

param(
    [string]$Source = (Join-Path $PSScriptRoot '..\ps4\sce_sys\icon0.png')
)

$ErrorActionPreference = 'Stop'
$out = Join-Path $PSScriptRoot 'sce_sys'
$tmp = Join-Path $env:TEMP 'jillex-livearea'
New-Item -ItemType Directory -Force -Path $tmp, (Join-Path $out 'livearea\contents') | Out-Null

if (-not (Get-Command ffmpeg -ErrorAction SilentlyContinue)) {
    throw 'ffmpeg was not found on PATH; it is needed for the 256-colour quantisation.'
}

function Convert-Indexed([int]$Width, [int]$Height, [string]$Dest) {
    $palette = Join-Path $tmp 'palette.png'
    ffmpeg -y -loglevel error -i $Source `
        -vf "scale=${Width}:${Height}:flags=lanczos,palettegen=max_colors=256:stats_mode=full" `
        -frames:v 1 $palette
    ffmpeg -y -loglevel error -i $Source -i $palette `
        -lavfi "scale=${Width}:${Height}:flags=lanczos[x];[x][1:v]paletteuse=dither=sierra2_4a" `
        -frames:v 1 -pix_fmt pal8 $Dest
    $size = (Get-Item $Dest).Length
    Write-Host ("{0,-28} {1}x{2} pal8 {3} bytes" -f (Split-Path $Dest -Leaf), $Width, $Height, $size)
}

Convert-Indexed 128 128 (Join-Path $out 'icon0.png')
Convert-Indexed 280 158 (Join-Path $out 'livearea\contents\startup.png')
Convert-Indexed 840 500 (Join-Path $out 'livearea\contents\bg.png')
