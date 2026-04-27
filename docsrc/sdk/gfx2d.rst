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

	xmc::Sprite xmc::createSprite565(int width, int height, XmcRamCap caps = XMC_RAM_CAP_DMA);
	xmc::Sprite xmc::createSprite565(int width, int height, void *data, uint32_t stride = 0, bool autoFree = false);

	xmc::Sprite xmc::createSprite444(int width, int height, XmcRamCap caps = XMC_RAM_CAP_DMA);
	xmc::Sprite xmc::createSprite444(int width, int height, void *data, uint32_t stride = 0, bool autoFree = false);

	xmc::Sprite xmc::createSprite4444(int width, int height, XmcRamCap caps = XMC_RAM_CAP_DMA);
	xmc::Sprite xmc::createSprite4444(int width, int height, void *data, uint32_t stride = 0, bool autoFree = false);

	xmc::Sprite xmc::createSpriteGray1(int width, int height, XmcRamCap caps = XMC_RAM_CAP_DMA);
	xmc::Sprite xmc::createSpriteGray1(int width, int height, void *data, uint32_t stride = 0, bool autoFree = false);

Sprite を生成します。
`data` 付きのオーバーロードでは外部バッファを使用できます。

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

xmc::createGraphics2D
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::Graphics2D xmc::createGraphics2D();
	xmc::Graphics2D xmc::createGraphics2D(xmc::Sprite target);

`Graphics2D` インスタンスを生成します。
`target` 指定版は描画先 Sprite を初期設定した状態で生成します。

xmc::Graphics2DClass::setTarget / getTarget
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics2DClass::setTarget(xmc::Sprite &s);
	xmc::Sprite xmc::Graphics2DClass::getTarget() const;

描画先 Sprite を設定・取得します。

xmc::Graphics2DClass::setClipRect / clearClipRect
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics2DClass::setClipRect(const xmc::Rect &rect);
	void xmc::Graphics2DClass::setClipRect(int x, int y, int w, int h);
	void xmc::Graphics2DClass::clearClipRect();

描画クリップ矩形を設定します。

xmc::Graphics2DClass::devColor
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::DevColor xmc::Graphics2DClass::devColor(int r, int g, int b, int a = 255);
	xmc::DevColor xmc::Graphics2DClass::devColor(xmc::Colors color);

描画対象のピクセルフォーマットに合わせて色を変換します。

xmc::Graphics2DClass の基本描画
--------------------------------------------------------------------------------

.. code-block:: cpp

	void clear(xmc::DevColor color);
	void fillRect(int x, int y, int w, int h, xmc::DevColor color);
	void drawRect(int x, int y, int w, int h, xmc::DevColor color);
	void fillSmokeRect(int x, int y, int w, int h, bool white = false);

矩形塗りつぶし、矩形枠、半透明風のスモーク塗りつぶしを行います。

xmc::Graphics2DClass::drawImage
--------------------------------------------------------------------------------

.. code-block:: cpp

	void drawImage(const xmc::Sprite &image, int dx, int dy, int w, int h, int sx, int sy, const xmc::TextRenderArgs &tra);
	void drawImage(const xmc::Sprite &image, int dx, int dy, int w, int h, int sx, int sy);
	void drawImage(const xmc::Sprite &image, int dx, int dy);

Sprite を描画します。
転送元矩形を指定する版と、画像全体を描画する簡易版があります。

xmc::Graphics2DClass のテキスト描画
--------------------------------------------------------------------------------

.. code-block:: cpp

	void setFont(const GFXfont *font);
	void setFont(const GFXfont *font, int size);
	void setFontSize(int size);
	void setCursor(int x, int y);
	void setTextColor(xmc::DevColor fg);
	void setTextColor(xmc::DevColor fg, xmc::DevColor bg);
	void drawString(const char *str);

フォント、描画位置、文字色を設定して文字列を描画します。

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

xmc::createFrameBuffer
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::FrameBuffer xmc::createFrameBuffer(xmc::PixelFormat fmt, bool doubleBuffered = false, int width = xmc::display::WIDTH, int height = xmc::display::HEIGHT);

フレームバッファを生成します。

xmc::FrameBufferClass の主な関数
--------------------------------------------------------------------------------

.. code-block:: cpp

	bool isDoubleBuffered() const;
	int getWidth() const;
	int getHeight() const;
	xmc::PixelFormat getPixelFormat() const;
	xmc::Sprite getFrontBuffer() const;
	xmc::Sprite getBackBuffer() const;
	xmc::Graphics2D createGraphics() const;

	XmcStatus beginRender();
	XmcStatus endRender(bool transferToDisplay = true);

	void enableFlag(xmc::FrameBufferFlags flag);
	void disableFlag(xmc::FrameBufferFlags flag);

`beginRender()` から `endRender()` までが 1 フレームの描画区間です。
`transferToDisplay` を `true` にすると描画内容がディスプレイへ転送されます。

