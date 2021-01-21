#!/bin/sh

#ffmpeg -r 30 -i frames/step%05d.png -c:v libx264 -vf fps=30 out.mp4

#ffmpeg -i out.mp4 -plays 0 -vf "fps=30,scale=1200:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse" -q:v 90 out.gif

#rm out.mp4

#palette="/tmp/palette.png"

#filters="fps=30,scale=600:-1:flags=lanczos"

#ffmpeg -v warning -i out.gif -vf "$filters,palettegen=stats_mode=diff" -y $palette

#ffmpeg -i out.gif -i $palette -lavfi "$filters,paletteuse=dither=bayer:bayer_scale=5:diff_mode=rectangle" -y final.gif

#rm out.gif


ffmpeg -framerate 60 -pattern_type glob -i 'frames/step*.png' -c:v libx264 -pix_fmt yuv420p out.mp4

