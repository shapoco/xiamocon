#!/usr/bin/env bash

_img2cpp_completion() {
    local cur prev words cword
    _init_completion || return

    case "$prev" in
        -f|--format)
            COMPREPLY=($(compgen -W "argb4444 rgb565 gray1" -- "$cur"))
            return
            ;;
        -d|--dither)
            COMPREPLY=($(compgen -W "none diffusion pattern" -- "$cur"))
            return
            ;;
        -k|--key-color)
            return
            ;;
    esac

    if [[ "$cur" == -* ]]; then
        COMPREPLY=($(compgen -W "-f --format -d --dither -k --key-color" -- "$cur"))
        return
    fi

    # ファイル補完 (画像ファイル / .hpp)
    COMPREPLY=($(compgen -f -- "$cur"))
}

complete -F _img2cpp_completion img2cpp
