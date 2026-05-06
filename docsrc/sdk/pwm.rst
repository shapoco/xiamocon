PWM API
################################################################################

概要
================================================================================

PWM API は、単一ピンのハードウェア PWM 出力を制御するための低レベル API です。

`xmc::pwm::createPwmDriver()` で生成して使用します。

設定
================================================================================

xmc::pwm::PwmConfig
--------------------------------------------------------------------------------

.. code-block:: cpp

	struct xmc::pwm::PwmConfig {
	  uint32_t freqHz;
	  uint32_t period;
	};

PWM 初期化時に使用する設定構造体です。

`freqHz` には PWM の周波数を Hz 単位で指定します。
`period` にはデューティサイクルの最大値（分解能）を指定します。

xmc::pwm::getDefaultPwmConfig
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::pwm::PwmConfig xmc::pwm::getDefaultPwmConfig();

推奨デフォルト値で初期化された `PwmConfig` を返します。
`PwmDriverClass::start()` に渡す設定値を取得するために使用します。

PwmDriver
================================================================================

PwmDriver は std::shared_ptr<PwmDriverClass> のエイリアスです。

xmc::pwm::createPwmDriver
--------------------------------------------------------------------------------

.. code-block:: cpp

	xmc::pwm::PwmDriver xmc::pwm::createPwmDriver(int pin = XMC_PIN_GPIO_0);

`PwmDriver` を生成して返します。
通常はこの関数でドライバを作成し、`start()` 後に `write()` を使用します。

start
--------------------------------------------------------------------------------

.. code-block:: cpp

	XmcStatus xmc::pwm::PwmDriverClass::start(const xmc::pwm::PwmConfig &cfg, float *actualFreqHz = nullptr);

PWM 信号の生成を開始します。

`cfg` には初期化設定を指定します。
`actualFreqHz` が `nullptr` でない場合は、実際に設定された周波数 [Hz] を書き込みます。

戻り値は `XmcStatus` で、成功時は `XMC_OK` です。

stop
--------------------------------------------------------------------------------

.. code-block:: cpp

	XmcStatus xmc::pwm::PwmDriverClass::stop();

PWM 信号の生成を停止します。
戻り値は `XmcStatus` で、成功時は `XMC_OK` です。

write
--------------------------------------------------------------------------------

.. code-block:: cpp

	XmcStatus xmc::pwm::PwmDriverClass::write(uint32_t cycle);

PWM のデューティサイクルを設定します。

`cycle` には 0 から `PwmConfig::period` の範囲でデューティサイクル値を指定します。
戻り値は `XmcStatus` で、成功時は `XMC_OK` です。
