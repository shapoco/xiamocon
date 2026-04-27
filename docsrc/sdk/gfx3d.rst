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

xmc::createNode3D
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::Node3D xmc::createNode3D(xmc::Mesh3D mesh, xmc::mat4 transform = xmc::mat4::identity());

メッシュとローカル変換行列からノードを生成します。

xmc::createMesh3D
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::Mesh3D xmc::createMesh3D(std::vector<xmc::Primitive3D> &&prims);

プリミティブ集合からメッシュを生成します。

xmc::createCube / createColoredCube / createSphere
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::Mesh3D xmc::createCube(float s = 1.0f, bool uv = true);
	xmc::Mesh3D xmc::createColoredCube(float s = 1.0f);
	xmc::Mesh3D xmc::createSphere(float radius = 1.0f, int segments = 12, int rings = 6, xmc::colorf col = {1.0f, 1.0f, 1.0f, 1.0f});

定型メッシュを生成します。

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

xmc::createMaterial3D
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::Material3D xmc::createMaterial3D();

マテリアルを生成します。

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

	xmc::Vec2Buffer xmc::createVec2Buffer(int size, XmcRamCap caps = XMC_RAM_CAP_SPIRAM);
	xmc::Vec2Buffer xmc::createVec2Buffer(xmc::vec2 *data, int size, bool autoFree = false);

	xmc::Vec3Buffer xmc::createVec3Buffer(int size, XmcRamCap caps = XMC_RAM_CAP_SPIRAM);
	xmc::Vec3Buffer xmc::createVec3Buffer(xmc::vec3 *data, int size, bool autoFree = false);

	xmc::ColorBuffer xmc::createColorBuffer(int size, XmcRamCap caps = XMC_RAM_CAP_SPIRAM);
	xmc::ColorBuffer xmc::createColorBuffer(xmc::colorf *data, int size, bool autoFree = false);

	xmc::IndexBuffer xmc::createIndexBuffer(int size, XmcRamCap caps = XMC_RAM_CAP_SPIRAM);
	xmc::IndexBuffer xmc::createIndexBuffer(uint16_t *data, int size, bool autoFree = false);

頂点属性配列を生成します。

Graphics3D
================================================================================

xmc::createGraphics3D
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::Graphics3D xmc::createGraphics3D(int width, int height, uint32_t stackSize = 16);

3D 描画コンテキストを生成します。

xmc::Graphics3DClass の主な設定関数
--------------------------------------------------------------------------------

.. code-block:: cpp

	void setTarget(xmc::Sprite target);
	void setTarget(xmc::Sprite target, xmc::Rect viewport);

	void setParallelMode(xmc::ParallelMode3D mode);
	xmc::ParallelMode3D getParallelMode() const;

	void setFlags(xmc::RenderFlags3D flags);
	void enableFlags(xmc::RenderFlags3D flags);
	void disableFlags(xmc::RenderFlags3D flags);

	void setBlendMode(xmc::BlendMode mode);
	void setEnvironmentLight(const xmc::colorf &color);
	void setParallelLight(const xmc::vec3 &dir, const xmc::colorf &color);

	void setProjection(const xmc::mat4 &mat);
	void setOrthoProjection(float left, float right, float bottom, float top, float near = 0.01f, float far = 100.0f);
	void setPerspectiveProjection(float fovY, float aspect, float near = 0.01f, float far = 100.0f);
	void setViewMatrix(const xmc::mat4 &mv);
	void lookAt(const xmc::vec3 &eye, const xmc::vec3 &focus, const xmc::vec3 &up);

xmc::Graphics3DClass の行列操作
--------------------------------------------------------------------------------

.. code-block:: cpp

	void pushState();
	void popState();
	void loadIdentity();
	void loadMatrix(const xmc::mat4 &m);
	void transform(const xmc::mat4 &t);
	void translate(const xmc::vec3 &t);
	void translate(float x, float y, float z);
	void rotate(const xmc::quat &q);
	void rotate(float pitch, float yaw, float roll);
	void rotate(const xmc::vec3 &axis, float angle);
	void scale(const xmc::vec3 &s);
	void scale(float s);

モデル変換スタックを操作します。

xmc::Graphics3DClass の描画関数
--------------------------------------------------------------------------------

.. code-block:: cpp

	void beginRender(xmc::ClearTarget target = xmc::ClearTarget::ALL);
	void render(const xmc::Scene3D &scene);
	void render(const xmc::Node3D &node);
	void render(const xmc::Mesh3D &mesh);
	void render(const xmc::Primitive3D &prim);
	void endRender();

`beginRender()` と `endRender()` の間で `render()` を呼び出して描画します。

