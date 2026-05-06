ADC API
################################################################################

概要
================================================================================

ADC API は、単一ピンの ADC 入力を初期化し、RAW 値または電圧値として読み取るための低レベル API です。

`xmc::adc::createAdcDriver()` で生成して使用します。

設定
================================================================================

xmc::adc::AdcConfig
--------------------------------------------------------------------------------

.. code-block:: cpp

	struct xmc::adc::AdcConfig {};

ADC 初期化時に使用する設定構造体です。
現行定義ではメンバーはありませんが、将来の設定拡張点として用意されています。

xmc::adc::getDefaultAdcConfig
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::adc::AdcConfig xmc::adc::getDefaultAdcConfig();

推奨デフォルト値で初期化された `AdcConfig` を返します。
`AdcDriverClass::init()` に渡す設定値を取得するために使用します。

AdcDriver
================================================================================

AdcDriver は std::shared_ptr<AdcDriverClass> のエイリアスです。

xmc::adc::createAdcDriver
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::adc::AdcDriver xmc::adc::createAdcDriver(int pin = XMC_PIN_GPIO_0);

`AdcDriver` を生成して返します。
通常はこの関数でドライバを作成し、`init()` 後に `readRaw()` / `readVoltage()` を使用します。


init
--------------------------------------------------------------------------------

.. code-block:: cpp

	XmcStatus xmc::adc::AdcDriverClass::init(const xmc::adc::AdcConfig &cfg);

ADC ドライバを初期化します。
`cfg` には初期化設定を指定します。戻り値は `XmcStatus` で、成功時は `XMC_OK` です。

deinit
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::adc::AdcDriverClass::deinit();

ADC ドライバを終了します。
以後 ADC を使用しない場合に呼び出します。

getMaxValue
--------------------------------------------------------------------------------

.. code-block:: cpp

	void xmc::adc::AdcDriverClass::getMaxValue(uint16_t *raw = nullptr, float *voltage = nullptr);

ADC の最大 RAW 値と最大電圧を取得します。

`raw` が `nullptr` でない場合は最大 RAW 値を書き込み、`voltage` が `nullptr` でない場合は基準となる最大電圧 [V] を書き込みます。

readRaw
--------------------------------------------------------------------------------

.. code-block:: cpp

	XmcStatus xmc::adc::AdcDriverClass::readRaw(uint16_t *value);

ADC を 1 回サンプリングし、RAW 値を取得します。
`value` に読み取り結果を書き込みます。戻り値は `XmcStatus` です。

readVoltage
--------------------------------------------------------------------------------

.. code-block:: cpp

	XmcStatus xmc::adc::AdcDriverClass::readVoltage(float *value);

ADC を 1 回サンプリングし、電圧値 [V] として取得します。
`value` に変換結果を書き込み、戻り値は `XmcStatus` です。
