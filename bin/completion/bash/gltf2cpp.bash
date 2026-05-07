#!/usr/bin/env bash

_gltf2cpp_completion() {
    local cur prev words cword
    _init_completion || return

    case "$prev" in
        -p|--prefix)
            return
            ;;
        -d|--dither)
            COMPREPLY=($(compgen -W "none diffusion pattern" -- "$cur"))
            return
            ;;
        -t|--texformat)
            COMPREPLY=($(compgen -W "rgb565 argb4444" -- "$cur"))
            return
            ;;
        -k|--key-color)
            return
            ;;
    esac

    if [[ "$cur" == -* ]]; then
        COMPREPLY=($(compgen -W "-p --prefix -d --dither -t --texformat -k --key-color" -- "$cur"))
        return
    fi

    # ファイル補完 (.gltf / .glb / .hpp)
    COMPREPLY=($(compgen -f -- "$cur"))
}

complete -F _gltf2cpp_completion gltf2cpp
