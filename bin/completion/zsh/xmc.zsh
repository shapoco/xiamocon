#compdef xmc

_xmc() {
    local -a platforms
    platforms=(rp2350_pico_sdk esp32s3_pio_arduino)

    local -a commands
    commands=(
        'init:プロジェクトを初期化する'
        'build:プロジェクトをビルドする'
        'run:プロジェクトをビルドして書き込む'
        'clean:ビルド成果物を削除する'
    )

    _arguments \
        '(-n --name)'{-n,--name}'[プロジェクト名を指定する]:name' \
        '(-p --platform)'{-p,--platform}'[プラットフォームを指定する]:platform:('"${platforms[*]}"')' \
        '(-d --drive)'{-d,--drive}'[書き込み先ドライブパスを指定する]:drive:_files -/' \
        '(-s --serial)'{-s,--serial}'[シリアルポートを指定する]:port:_files -g "/dev/ttyUSB* /dev/ttyACM*"' \
        ':command:(('"${commands[*]}"'))'
}

compdef _xmc xmc
