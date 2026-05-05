2D グラフィックス API
################################################################################

概要
================================================================================

2D 描画は主に次の 3 つの要素で構成されます。

- Sprite: 画像バッファ
- Graphics2D: Sprite への描画操作
- FrameBuffer: 画面転送を含む描画フレーム管理

ピクセルフォーマット
================================================================================

xmc::PixelFormat
--------------------------------------------------------------------------------

.. csv-table:: xmc::PixelFormat の値
	 :header: "値", "説明"

	 "xmc::PixelFormat::GRAY1", "1bit グレイスケール"
	 "xmc::PixelFormat::RGB444", "12bit RGB"
	 "xmc::PixelFormat::ARGB4444", "16bit ARGB"
	 "xmc::PixelFormat::RGB565", "16bit RGB"

xmc::Colors
--------------------------------------------------------------------------------

代表的な色定数です。

.. csv-table:: xmc::Colors の主な値
	 :header: "値", "説明"

	 "xmc::Colors::BLACK", "黒"
	 "xmc::Colors::WHITE", "白"
	 "xmc::Colors::RED", "赤"
	 "xmc::Colors::GREEN", "緑"
	 "xmc::Colors::BLUE", "青"
	 "xmc::Colors::YELLOW", "黄"
	 "xmc::Colors::CYAN", "シアン"
	 "xmc::Colors::MAGENTA", "マゼンタ"

Sprite
================================================================================

xmc::createSprite565 / 444 / 4444 / Gray1
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::Sprite xmc::createSprite565(int width, int height, XmcHeapCap caps = XMC_HEAP_CAP_DMA);
	xmc::Sprite xmc::createSprite565(int width, int height, void *data, uint32_t stride = 0, bool autoFree = false);

	xmc::Sprite xmc::createSprite444(int width, int height, XmcHeapCap caps = XMC_HEAP_CAP_DMA);
	xmc::Sprite xmc::createSprite444(int width, int height, void *data, uint32_t stride = 0, bool autoFree = false);

	xmc::Sprite xmc::createSprite4444(int width, int height, XmcHeapCap caps = XMC_HEAP_CAP_DMA);
	xmc::Sprite xmc::createSprite4444(int width, int height, void *data, uint32_t stride = 0, bool autoFree = false);

	xmc::Sprite xmc::createSpriteGray1(int width, int height, XmcHeapCap caps = XMC_HEAP_CAP_DMA);
	xmc::Sprite xmc::createSpriteGray1(int width, int height, void *data, uint32_t stride = 0, bool autoFree = false);

Sprite を生成します。スプライトのピクセルフォーマットに応じた関数を使用します。

width と height にはスプライトのサイズをピクセル単位で指定します。

RAM 上にスプライトを作成する場合は、`caps` に RAM の種類を指定します。
スプライトを DMA 転送に使用する場合は `XMC_HEAP_CAP_DMA` を指定しなければなりません。

RAM 上に既に確保されたバッファや Flash ROM 上のリソースをスプライトとして使用する場合は、`data` 引数にバッファの先頭アドレスを指定します。
この場合、`stride` 引数に 1 ラインあたりのバイト数を指定します。0 を指定した場合は、幅に応じた値が自動計算されます。
`autoFree` を `true` にすると、スプライトオブジェクトの寿命が尽きたときに `data` で指定されたバッファを自動的に解放します。

xmc::SpriteClass
--------------------------------------------------------------------------------

Sprite の実体クラスです。`xmc::Sprite` は `std::shared_ptr<xmc::SpriteClass>` の別名です。

.. csv-table:: xmc::SpriteClass の主なメンバー
	 :header: "型", "メンバー", "説明"

	 "xmc::PixelFormat", "format", "ピクセルフォーマット"
	 "int", "width", "画像幅"
	 "int", "height", "画像高さ"
	 "uint32_t", "stride", "1 ラインあたりのバイト数"
	 "void *", "data", "ピクセルデータ先頭ポインタ"

xmc::SpriteClass::linePtr
--------------------------------------------------------------------------------

.. code-block:: cpp

	void *xmc::SpriteClass::linePtr(int y) const;

指定行の先頭ポインタを取得します。範囲外の `y` を指定した場合は `nullptr` を返します。

Graphics2D
================================================================================

コンストラクタ
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::Graphics2D xmc::createGraphics2D();
	xmc::Graphics2D xmc::createGraphics2D(xmc::Sprite target);

`Graphics2D` インスタンスを生成します。
`target` 指定版は描画先 Sprite を初期設定した状態で生成します。

引数を指定しなかった場合、スプライトではなくディスプレイへ直接描画されます。

戻り値として `xmc::Graphics2D` オブジェクトを返します。

setTarget / getTarget
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics2DClass::setTarget(xmc::Sprite &s);
	xmc::Sprite xmc::Graphics2DClass::getTarget() const;

描画先 Sprite を設定・取得します。

`setTarget()` の引数 `s` には、新しい描画先として使用する Sprite を指定します。
`getTarget()` は現在設定されている描画先 Sprite を返します。

setClipRect / clearClipRect
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics2DClass::setClipRect(const xmc::Rect &rect);
	void xmc::Graphics2DClass::setClipRect(int x, int y, int w, int h);
	void xmc::Graphics2DClass::clearClipRect();

描画クリップ矩形を設定します。

`setClipRect(const Rect &rect)` では矩形構造体で描画範囲を指定します。
`setClipRect(int x, int y, int w, int h)` では左上座標とサイズを個別に指定します。
クリップ範囲の外側には描画されません。

`clearClipRect()` を呼ぶとクリップ矩形が解除され、描画先全体に描画できる状態に戻ります。

devColor
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::DevColor xmc::Graphics2DClass::devColor(int r, int g, int b, int a = 255);
	xmc::DevColor xmc::Graphics2DClass::devColor(xmc::Colors color);

描画対象のピクセルフォーマットに合わせて色を変換します。

`devColor(int r, int g, int b, int a = 255)` では 8bit 整数の RGBA 値を指定します。
各成分は内部で描画先のピクセルフォーマットに合わせて変換されます。

`devColor(xmc::Colors color)` ではあらかじめ定義された色定数を使用できます。

戻り値は、そのまま描画関数に渡せる `xmc::DevColor` です。

devColorHSV
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::DevColor xmc::Graphics2DClass::devColorHSV(int h, int s, int v);

HSV 色空間で色を指定し、描画対象のピクセルフォーマットに合わせて変換します。

`h` は色相を 0〜359 の整数で指定します。範囲外の値は自動的に正規化されます。
`s` は彩度、`v` は明度をそれぞれ 0〜255 の整数で指定します。

戻り値は、そのまま描画関数に渡せる `xmc::DevColor` です。

clear
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics2DClass::clear(xmc::DevColor color);

現在のクリップ範囲全体を `color` で塗りつぶします。

setPixel
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics2DClass::setPixel(int x, int y, xmc::DevColor color);

1 ピクセルを描画します。

fillRect
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics2DClass::fillRect(xmc::Rect dstRect, xmc::DevColor color);
	void xmc::Graphics2DClass::fillRect(int x, int y, int w, int h, xmc::DevColor color);

矩形内部を塗りつぶします。
`xmc::Rect` 構造体で指定する版と、座標とサイズを個別に指定する版があります。

drawRect
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics2DClass::drawRect(xmc::Rect dstRect, xmc::DevColor color);
	void xmc::Graphics2DClass::drawRect(int x, int y, int w, int h, xmc::DevColor color);

矩形の枠線のみを描画します。
`xmc::Rect` 構造体で指定する版と、座標とサイズを個別に指定する版があります。

fillEllipse
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics2DClass::fillEllipse(xmc::Rect dstRect, xmc::DevColor color);
	void xmc::Graphics2DClass::fillEllipse(int x, int y, int w, int h, xmc::DevColor color);

`dstRect` で指定された矩形に内接する楕円を `color` で塗りつぶします。
`xmc::Rect` 構造体で指定する版と、座標とサイズを個別に指定する版があります。

fillSmokeRect
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics2DClass::fillSmokeRect(xmc::Rect dstRect, bool white = false);
	void xmc::Graphics2DClass::fillSmokeRect(int x, int y, int w, int h, bool white = false);

白または黒ベースのスモーク表現を描画する補助関数です。
`white` を `true` にすると各ピクセルを現在の色と白の中間色で塗りつぶし、
`false` の場合は現在の色と黒の中間色で塗りつぶします。

drawLine
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics2DClass::drawLine(int x1, int y1, int x2, int y2, xmc::DevColor color);

2 点間に直線を描画します。

drawImage
--------------------------------------------------------------------------------

.. code-block:: cpp

	void drawImage(const xmc::Sprite &image, int dx, int dy, int w, int h, int sx, int sy, const xmc::TextRenderArgs &tra);
	void drawImage(const xmc::Sprite &image, int dx, int dy, int w, int h, int sx, int sy);
	void drawImage(const xmc::Sprite &image, int dx, int dy);

Sprite を描画します。
転送元矩形を指定する版と、画像全体を描画する簡易版があります。

`image` には描画元 Sprite を指定します。
`dx`, `dy` は描画先の左上座標です。

完全版では、`w`, `h` にコピーサイズ、`sx`, `sy` に描画元 Sprite 内の開始座標を指定します。
`tra` には前景色・背景色・描画フラグを指定し、1bit 画像やフォント画像の描画方法を制御できます。

2 引数版は画像全体をそのまま描画し、7 引数版は `tra` に既定値を使って部分描画します。

setFont
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics2DClass::setFont(const GFXfont *font);
	void xmc::Graphics2DClass::setFont(const GFXfont *font, int size);

文字描画に使用するフォントを設定します。
サイズ付きオーバーロードでは `size` に拡大倍率を指定します。

setFontSize
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics2DClass::setFontSize(int size);

現在設定されているフォントの描画サイズ（拡大倍率）を設定します。

setCursor
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics2DClass::setCursor(int x, int y);

文字列描画の開始位置を設定します。

setTextColor
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics2DClass::setTextColor(xmc::DevColor fg);
	void xmc::Graphics2DClass::setTextColor(xmc::DevColor fg, xmc::DevColor bg);

文字色を設定します。
`setTextColor(fg)` は前景色のみを設定し、
`setTextColor(fg, bg)` は前景色と背景色の両方を設定します。

drawString
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics2DClass::drawString(const char *str);

現在のフォント設定、文字色、カーソル位置を使って文字列を描画します。

FrameBuffer
================================================================================

xmc::FrameBufferFlags
--------------------------------------------------------------------------------

.. csv-table:: xmc::FrameBufferFlags の値
	 :header: "値", "説明"

	 "xmc::FrameBufferFlags::SHOW_STATUS_BAR", "ステータスバー表示"
	 "xmc::FrameBufferFlags::SHOW_BATTERY_LEVEL", "バッテリー表示"
	 "xmc::FrameBufferFlags::SHOW_FPS", "FPS 表示"
	 "xmc::FrameBufferFlags::SHOW_DEBUG_INFO", "デバッグ情報表示"
	 "xmc::FrameBufferFlags::DEFAULT", "標準フラグ"

コンストラクタ
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::FrameBuffer xmc::createFrameBuffer(xmc::PixelFormat fmt, bool doubleBuffered = false, int width = xmc::display::WIDTH, int height = xmc::display::HEIGHT);

フレームバッファを生成します。

`fmt` にはバッファのピクセルフォーマットを指定します。
`doubleBuffered` を `true` にするとダブルバッファリングが有効になります。
`width`, `height` を省略した場合はディスプレイサイズが使用されます。

戻り値として、描画制御と画面転送を扱う `xmc::FrameBuffer` オブジェクトを返します。

isDoubleBuffered
--------------------------------------------------------------------------------

.. code-block:: cpp

	bool xmc::FrameBufferClass::isDoubleBuffered() const;

ダブルバッファリングが有効かどうかを返します。
有効な場合は `true`、シングルバッファの場合は `false` を返します。

getWidth / getHeight
--------------------------------------------------------------------------------

.. code-block:: cpp

	int xmc::FrameBufferClass::getWidth() const;
	int xmc::FrameBufferClass::getHeight() const;

フレームバッファのサイズを返します。
戻り値はそれぞれピクセル単位の幅と高さです。

getPixelFormat
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::PixelFormat xmc::FrameBufferClass::getPixelFormat() const;

フレームバッファのピクセルフォーマットを返します。
戻り値は `xmc::PixelFormat` 列挙型です。

getFrontBuffer / getBackBuffer
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::Sprite xmc::FrameBufferClass::getFrontBuffer() const;
	xmc::Sprite xmc::FrameBufferClass::getBackBuffer() const;

フロントバッファとバックバッファを取得します。

`getFrontBuffer()` は現在表示中のバッファを返します。
`getBackBuffer()` は次フレーム描画用のバッファを返します。

シングルバッファ構成では、両者が同じ実体を指す場合があります。

createGraphics
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::Graphics2D xmc::FrameBufferClass::createGraphics() const;

バックバッファを描画先に設定した `Graphics2D` オブジェクトを生成して返します。
フレームバッファへ 2D 描画を行う際の基本的な入口として使用します。

beginRender / endRender
--------------------------------------------------------------------------------

.. code-block:: cpp

	XmcStatus xmc::FrameBufferClass::beginRender();
	XmcStatus xmc::FrameBufferClass::endRender(bool transferToDisplay = true);

1 フレーム分の描画区間を開始・終了します。

`beginRender()` は描画開始前の準備を行います。
`endRender()` は描画終了処理を行い、`transferToDisplay` が `true` の場合は描画内容をディスプレイへ転送します。

戻り値はいずれも `XmcStatus` で、正常に処理できた場合は `XMC_OK` が返ります。

enableFlag / disableFlag
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::FrameBufferClass::enableFlag(xmc::FrameBufferFlags flag);
	void xmc::FrameBufferClass::disableFlag(xmc::FrameBufferFlags flag);

フレームバッファの表示補助機能を有効化・無効化します。

`flag` には `xmc::FrameBufferFlags` の値を指定します。
ステータスバー、バッテリー残量、FPS 表示などの付加表示を切り替える用途で使用します。

FpsKeeper
================================================================================

描画ループを目標 FPS に同期するための補助クラスです。

コンストラクタ
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::FpsKeeper::FpsKeeper(float fps, uint32_t maxJitter = 5000);

`fps` には目標フレームレートを指定します。
`maxJitter` は遅延許容値をマイクロ秒単位で指定します。

`waitVsync()` 呼び出し時に目標時刻まで待機し、
遅延が `maxJitter` を超えた場合は内部の同期時刻を現在時刻に再同期します。

setTargetFps
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::FpsKeeper::setTargetFps(float fps);

実行中に目標 FPS を変更します。

waitVsync
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::FpsKeeper::waitVsync();

次フレームの目標タイミングまで待機します。

内部では、前回までの基準時刻と目標 FPS から次回時刻を計算します。
処理が遅延している場合はフレームスキップ状態を更新し、
必要に応じて同期時刻を補正します。

isFrameSkipping
--------------------------------------------------------------------------------

.. code-block:: cpp

	bool xmc::FpsKeeper::isFrameSkipping() const;

現在フレームをスキップすべき状態かどうかを返します。

戻り値が `true` のフレームでは描画負荷の高い処理を省略し、
`false` のフレームで通常描画を行うことで、遅延からの回復を支援できます。

