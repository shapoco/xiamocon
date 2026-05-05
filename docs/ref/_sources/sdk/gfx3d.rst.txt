3D グラフィックス API
################################################################################

概要
================================================================================

3D 描画は次の流れで使用します。

1. `Graphics3D` を生成して描画先を設定する
2. `Scene3D` / `Node3D` / `Mesh3D` / `Primitive3D` を組み立てる
3. `beginRender()` → `render()` → `endRender()` で 1 フレーム描画する

描画制御
================================================================================

xmc::ClearTarget
--------------------------------------------------------------------------------

`beginRender()` 時にクリア対象を指定する列挙型です。

.. csv-table:: xmc::ClearTarget の値
	 :header: "値", "説明"

	 "xmc::ClearTarget::STACK", "内部状態スタックをクリア"
	 "xmc::ClearTarget::DEPTH", "深度バッファをクリア"
	 "xmc::ClearTarget::ALL", "全クリア"

xmc::RenderFlags3D
--------------------------------------------------------------------------------

3D レンダリングの機能フラグです。

.. csv-table:: xmc::RenderFlags3D の主な値
	 :header: "値", "説明"

	 "xmc::RenderFlags3D::VERTEX_COLOR", "頂点カラーを使用"
	 "xmc::RenderFlags3D::VERTEX_NORMAL", "頂点法線を使用"
	 "xmc::RenderFlags3D::GOURAUD_SHADING", "グーローシェーディング"
	 "xmc::RenderFlags3D::LIGHTING", "ライティング"
	 "xmc::RenderFlags3D::COLOR_TEXTURE", "テクスチャカラー"
	 "xmc::RenderFlags3D::Z_TEST", "深度テスト"
	 "xmc::RenderFlags3D::Z_UPDATE", "深度バッファ更新"
	 "xmc::RenderFlags3D::DEFAULT", "標準設定"

xmc::ParallelMode3D
--------------------------------------------------------------------------------

並列レンダリングモードです。

.. csv-table:: xmc::ParallelMode3D の値
	 :header: "値", "説明"

	 "xmc::ParallelMode3D::NONE", "並列化なし"
	 "xmc::ParallelMode3D::INTERLACE", "インターレース分担"
	 "xmc::ParallelMode3D::PIPELINE", "パイプライン分担"

シーン構築
================================================================================

xmc::createScene3D
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::Scene3D xmc::createScene3D();

シーンを生成します。`Scene3DClass::rootNodes` または `addNode()` でノードを追加します。

戻り値として空の `xmc::Scene3D` オブジェクトを返します。
複数のルートノードを持つシーンを構築したい場合の起点になります。

xmc::createNode3D
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::Node3D xmc::createNode3D(xmc::Mesh3D mesh, xmc::mat4 transform = xmc::mat4::identity());

メッシュとローカル変換行列からノードを生成します。

`mesh` には描画対象メッシュを指定します。
`transform` にはノードに適用するローカル変換行列を指定します。

戻り値として `xmc::Node3D` オブジェクトを返します。
子ノードを `children` に追加することでシーングラフを構築できます。

xmc::createMesh3D
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::Mesh3D xmc::createMesh3D(std::vector<xmc::Primitive3D> &&prims);

プリミティブ集合からメッシュを生成します。

`prims` にはメッシュを構成するプリミティブ列を指定します。
戻り値として `xmc::Mesh3D` を返します。

xmc::createCube / createColoredCube / createSphere
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::Mesh3D xmc::createCube(float s = 1.0f, bool uv = true);
	xmc::Mesh3D xmc::createColoredCube(float s = 1.0f);
	xmc::Mesh3D xmc::createSphere(float radius = 1.0f, int segments = 12, int rings = 6, xmc::colorf col = {1.0f, 1.0f, 1.0f, 1.0f});

定型メッシュを生成します。

`createCube()` の `s` は立方体の一辺の長さです。
`uv` を `true` にすると UV 座標付きメッシュを生成します。

`createColoredCube()` は頂点色付き立方体を返します。

`createSphere()` では `radius` に半径、`segments` に経度方向分割数、`rings` に緯度方向分割数、`col` に球の基本色を指定します。

いずれも戻り値は `xmc::Mesh3D` です。

プリミティブとマテリアル
================================================================================

xmc::createPrimitive3D
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::Primitive3D xmc::createPrimitive3D(
			xmc::PrimitiveMode mode,
			xmc::Vec3Buffer pos,
			xmc::Vec3Buffer norm,
			xmc::ColorBuffer col,
			xmc::Vec2Buffer uv = nullptr,
			xmc::IndexBuffer idx = nullptr,
			xmc::Material3D mat = nullptr);

1 つの描画プリミティブを生成します。

`mode` には描画トポロジを指定します。
`pos` は頂点位置配列、`norm` は法線配列、`col` は頂点カラー配列です。
`uv` はテクスチャ座標、`idx` はインデックス配列、`mat` はマテリアルを指定します。

使用しない属性は `nullptr` を指定できますが、レンダリングフラグやマテリアル設定と整合している必要があります。
戻り値は `xmc::Primitive3D` です。

xmc::createMaterial3D
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::Material3D xmc::createMaterial3D();

マテリアルを生成します。

戻り値として、各種見た目設定を書き込むための `xmc::Material3D` オブジェクトを返します。

xmc::MaterialFlags3D
--------------------------------------------------------------------------------

.. csv-table:: xmc::MaterialFlags3D の値
	 :header: "値", "説明"

	 "xmc::MaterialFlags3D::HAS_BASE_COLOR", "ベースカラーを有効化"
	 "xmc::MaterialFlags3D::DOUBLE_SIDED", "両面描画"
	 "xmc::MaterialFlags3D::ENVIRONMENT_MAPPED", "環境マップ反射"

xmc::Material3DClass の主なメンバー
--------------------------------------------------------------------------------

.. csv-table:: xmc::Material3DClass の主なメンバー
	 :header: "型", "メンバー", "説明"

	 "xmc::MaterialFlags3D", "flags", "マテリアルフラグ"
	 "xmc::colorf", "baseColor", "ベースカラー"
	 "xmc::Sprite", "colorTexture", "カラーテクスチャ"
	 "xmc::VertexShader *", "vertexShader", "頂点シェーダ"

属性バッファ
================================================================================

.. code-block:: cpp

	xmc::Vec2Buffer xmc::createVec2Buffer(int size, XmcHeapCap caps = XMC_HEAP_CAP_SPIRAM);
	xmc::Vec2Buffer xmc::createVec2Buffer(xmc::vec2 *data, int size, bool autoFree = false);

	xmc::Vec3Buffer xmc::createVec3Buffer(int size, XmcHeapCap caps = XMC_HEAP_CAP_SPIRAM);
	xmc::Vec3Buffer xmc::createVec3Buffer(xmc::vec3 *data, int size, bool autoFree = false);

	xmc::ColorBuffer xmc::createColorBuffer(int size, XmcHeapCap caps = XMC_HEAP_CAP_SPIRAM);
	xmc::ColorBuffer xmc::createColorBuffer(xmc::colorf *data, int size, bool autoFree = false);

	xmc::IndexBuffer xmc::createIndexBuffer(int size, XmcHeapCap caps = XMC_HEAP_CAP_SPIRAM);
	xmc::IndexBuffer xmc::createIndexBuffer(uint16_t *data, int size, bool autoFree = false);

頂点属性配列を生成します。

サイズ指定版では指定要素数のバッファを新規確保して返します。
`caps` には確保先 RAM の特性を指定します。

ポインタ指定版では既存配列を属性バッファとしてラップします。
`data` は配列先頭、`size` は要素数です。
`autoFree` を `true` にするとオブジェクト破棄時に `data` が自動解放されます。

Graphics3D
================================================================================

xmc::createGraphics3D
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::Graphics3D xmc::createGraphics3D(int width, int height, uint32_t stackSize = 16);

3D 描画コンテキストを生成します。

`width`, `height` には描画先サイズをピクセル単位で指定します。
`stackSize` は変換状態を保持するスタック深さです。

戻り値として `xmc::Graphics3D` オブジェクトを返します。

setTarget
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics3DClass::setTarget(xmc::Sprite target);
	void xmc::Graphics3DClass::setTarget(xmc::Sprite target, xmc::Rect viewport);

3D 描画先 Sprite を設定します。

`target` には描画先 Sprite を指定します。
`viewport` 指定版では、Sprite 内の描画領域を矩形で指定できます。
ビューポート省略版では Sprite 全体が描画領域になります。

setParallelMode / getParallelMode
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics3DClass::setParallelMode(xmc::ParallelMode3D mode);
	xmc::ParallelMode3D xmc::Graphics3DClass::getParallelMode() const;

並列レンダリングモードを設定・取得します。

`mode` には `xmc::ParallelMode3D` の値を指定します。
`getParallelMode()` は現在の並列レンダリングモードを返します。

setFlags / enableFlags / disableFlags
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics3DClass::setFlags(xmc::RenderFlags3D flags);
	void xmc::Graphics3DClass::enableFlags(xmc::RenderFlags3D flags);
	void xmc::Graphics3DClass::disableFlags(xmc::RenderFlags3D flags);

レンダリング機能フラグを設定します。

`setFlags()` はフラグ全体を置き換えます。
`enableFlags()` と `disableFlags()` は指定したフラグだけを個別に有効化・無効化します。

setBlendMode
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics3DClass::setBlendMode(xmc::BlendMode mode);

ブレンドモードを設定します。
`mode` には `xmc::BlendMode` の値を指定します。

setEnvironmentLight / setParallelLight
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics3DClass::setEnvironmentLight(const xmc::colorf &color);
	void xmc::Graphics3DClass::setParallelLight(const xmc::vec3 &dir, const xmc::colorf &color);

ライティング条件を設定します。

`setEnvironmentLight()` は環境光の色を設定します。
`setParallelLight()` は平行光源の方向 `dir` と色 `color` を設定します。

setProjection / setOrthoProjection / setPerspectiveProjection
--------------------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics3DClass::setProjection(const xmc::mat4 &mat);
	void xmc::Graphics3DClass::setOrthoProjection(float left, float right, float bottom, float top, float near = 0.01f, float far = 100.0f);
	void xmc::Graphics3DClass::setPerspectiveProjection(float fovY, float aspect, float near = 0.01f, float far = 100.0f);

投影行列を設定します。

`setProjection()` は投影行列を直接設定します。
`setOrthoProjection()` は平行投影、`setPerspectiveProjection()` は透視投影を簡易に設定する補助関数です。

setViewMatrix / lookAt
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics3DClass::setViewMatrix(const xmc::mat4 &mv);
	void xmc::Graphics3DClass::lookAt(const xmc::vec3 &eye, const xmc::vec3 &focus, const xmc::vec3 &up);

ビュー行列を設定します。

`setViewMatrix()` はビュー行列を直接設定します。
`lookAt()` は視点 `eye`、注視点 `focus`、上方向 `up` からビュー行列を生成して設定します。

pushState / popState
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics3DClass::pushState();
	void xmc::Graphics3DClass::popState();

変換状態スタックを保存・復元します。
階層構造をたどりながら個別の変換を適用したい場合に使用します。

loadIdentity / loadMatrix / transform
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics3DClass::loadIdentity();
	void xmc::Graphics3DClass::loadMatrix(const xmc::mat4 &m);
	void xmc::Graphics3DClass::transform(const xmc::mat4 &t);

現在のモデル変換行列を設定します。

`loadIdentity()` は単位行列へ初期化します。
`loadMatrix()` は行列を直接設定します。
`transform()` は現在行列に変換行列を乗算します。

translate / rotate / scale
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics3DClass::translate(const xmc::vec3 &t);
	void xmc::Graphics3DClass::translate(float x, float y, float z);
	void xmc::Graphics3DClass::rotate(const xmc::quat &q);
	void xmc::Graphics3DClass::rotate(float pitch, float yaw, float roll);
	void xmc::Graphics3DClass::rotate(const xmc::vec3 &axis, float angle);
	void xmc::Graphics3DClass::scale(const xmc::vec3 &s);
	void xmc::Graphics3DClass::scale(float s);

現在のモデル変換に平行移動、回転、拡大縮小を適用します。
オーバーロードによりベクトル指定と成分指定の両方を使用できます。

beginRender / endRender
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics3DClass::beginRender(xmc::ClearTarget target = xmc::ClearTarget::ALL);
	void xmc::Graphics3DClass::endRender();

1 フレーム分の 3D 描画を開始・終了します。

`beginRender()` の `target` にはクリア対象を指定します。
通常は既定値の `xmc::ClearTarget::ALL` を使用します。

render
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::Graphics3DClass::render(const xmc::Scene3D &scene);
	void xmc::Graphics3DClass::render(const xmc::Node3D &node);
	void xmc::Graphics3DClass::render(const xmc::Mesh3D &mesh);
	void xmc::Graphics3DClass::render(const xmc::Primitive3D &prim);

シーンまたは描画要素をレンダリングします。

大きなシーン全体を描く場合は `Scene3D` を、個別に制御したい場合は `Node3D`、`Mesh3D`、`Primitive3D` を直接渡します。
`beginRender()` と `endRender()` の間で呼び出します。

