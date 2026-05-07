#compdef img2cpp

_img2cpp() {
    _arguments \
        '(-f --format)'{-f,--format}'[出力ピクセルフォーマットを指定する]:format:(argb4444 rgb565 gray1)' \
        '(-d --dither)'{-d,--dither}'[ディザリング方式を指定する]:dither:(none diffusion pattern)' \
        '(-k --key-color)'{-k,--key-color}'[透明にするキーカラーを指定する (例: \#FF8000, orange, F80)]:key-color' \
        ':input:_files -g "*.png *.jpg *.jpeg *.bmp *.gif *.webp"' \
        ':output:_files -g "*.hpp"'
}

compdef _img2cpp img2cpp
