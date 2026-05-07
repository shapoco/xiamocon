#compdef gltf2cpp

_gltf2cpp() {
    _arguments \
        '(-p --prefix)'{-p,--prefix}'[C++ 識別子のプレフィックスを指定する]:prefix' \
        '(-d --dither)'{-d,--dither}'[ディザリング方式を指定する]:dither:(none diffusion pattern)' \
        '(-t --texformat)'{-t,--texformat}'[テクスチャのピクセルフォーマットを指定する]:texformat:(rgb565 argb4444)' \
        '(-k --key-color)'{-k,--key-color}'[透明にするキーカラーを指定する (例: \#FF8000, orange, F80)]:key-color' \
        ':input:_files -g "*.gltf *.glb"' \
        ':output:_files -g "*.hpp"'
}

compdef _gltf2cpp gltf2cpp
