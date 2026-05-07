#!/usr/bin/env bash

_xmc_completion() {
    local cur prev words cword
    _init_completion || return

    local commands="init build run clean"
    local platforms="rp2350_pico_sdk esp32s3_pio_arduino"

    case "$prev" in
        -p|--platform)
            COMPREPLY=($(compgen -W "$platforms" -- "$cur"))
            return
            ;;
        -d|--drive)
            return
            ;;
        -s|--serial)
            local ports
            ports=$(ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null)
            COMPREPLY=($(compgen -W "$ports" -- "$cur"))
            return
            ;;
        -n|--name)
            return
            ;;
    esac

    if [[ "$cur" == -* ]]; then
        COMPREPLY=($(compgen -W "-n --name -p --platform -d --drive -s --serial" -- "$cur"))
        return
    fi

    # コマンドがまだ指定されていなければコマンド候補を出す
    local cmd=""
    for word in "${words[@]}"; do
        if [[ "$word" == @(init|build|run|clean) ]]; then
            cmd="$word"
            break
        fi
    done

    if [[ -z "$cmd" ]]; then
        COMPREPLY=($(compgen -W "$commands" -- "$cur"))
    fi
}

complete -F _xmc_completion xmc
