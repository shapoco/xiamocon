オーディオ API
################################################################################

グローバル関数
================================================================================

xmc::audio::setMuted
--------------------------------------------------------------------------------

.. code-block:: cpp

  XmcStatus xmc::audio::setMuted(bool muted);

スピーカーのミュート状態を設定します。
この関数は IO エキスパンダを経由してパワーアンプのミュート端子を制御します。
true を指定するとスピーカーがミュートされ、false を指定するとミュートが解除されます。

処理が成功すると XMC_OK が返されます。処理に失敗した場合は、エラーコードが返されます。

xmc::audio::setSourcePort
--------------------------------------------------------------------------------

.. code-block:: cpp

  XmcStatus xmc::audio::setSourcePort(xmc::audio::SourcePort *src);

オーディオストリームのソースを指定します。通常は xmc::audio::Mixer クラスから取得したオーディオソースを指定します。

Mixer クラス
================================================================================

xmc::audio::Mixer クラスは、複数のオーディオソースをミックスして単一のオーディオストリームを生成するためのクラスです。

xmc::audio::createMixer
--------------------------------------------------------------------------------

.. code-block:: cpp

  xmc::audio::Mixer xmc::audio::createMixer(int numSources);

Mixer クラスのインスタンスを作成します。引数にはミックスするオーディオソースの数を指定します。

xmc::audio::Mixer::setSource
--------------------------------------------------------------------------------

.. code-block:: cpp

  void xmc::audio::Mixer::setSource(int index, xmc::audio::SourcePort *source)

ミックスするオーディオソースを指定します。index 引数は 0 から numSources-1 の範囲で、source 引数にはオーディオソースを指定します。

xmc::audio::Mixer::getOutputPort
--------------------------------------------------------------------------------

.. code-block:: cpp

  xmc::audio::SourcePort *xmc::audio::Mixer::getOutputPort()

ミックスされたオーディオストリームの出力ポートを取得します。
この出力ポートを xmc::audio::setSourcePort() 関数に渡すことで、ミックスされたオーディオをスピーカーに送ることができます。

Toneクラス
================================================================================

xmc::audio::Tone クラスは、シンプルなトーンジェネレータを実装したクラスです。
簡単な効果音の生成に使用できます。

xmc::audio::createTone
--------------------------------------------------------------------------------

.. code-block:: cpp

  xmc::audio::Tone xmc::audio::createTone();

Tone クラスのインスタンスを作成します。引数はありません。

xmc::audio::Tone::init
--------------------------------------------------------------------------------

.. code-block:: cpp

  void xmc::audio::Tone::init(uint32_t rateHz = 0)

トーンジェネレータを初期化します。引数にはサンプルレートを指定します。
0 を指定した場合は、スピーカーのサンプルレートが使用されます。

xmc::audio::Waveform
--------------------------------------------------------------------------------

Tone で使用する波形を表す列挙型です。

.. csv-table:: xmc::audio::Waveform の値
   :header: "値", "説明"

   "xmc::audio::Waveform::SQUARE", "矩形波"
   "xmc::audio::Waveform::SINE", "正弦波"
   "xmc::audio::Waveform::TRIANGLE", "三角波"
   "xmc::audio::Waveform::SAWTOOTH", "のこぎり波"
   "xmc::audio::Waveform::NOISE", "ノイズ"

xmc::audio::TONE_LENGTH_INFINITE
--------------------------------------------------------------------------------

.. code-block:: cpp

  static constexpr uint32_t xmc::audio::TONE_LENGTH_INFINITE = 0xFFFFFFFF;

音の再生長を「無限」にするための定数です。
`noteOn()` / `noteOnWithFreq()` の `lenMs` 引数に指定すると、`noteOff()` または `mute()` が呼ばれるまで再生を継続します。

xmc::audio::Tone::setWaveform
--------------------------------------------------------------------------------

.. code-block:: cpp

  void xmc::audio::Tone::setWaveform(xmc::audio::Waveform wf);

出力波形を設定します。

xmc::audio::Tone::getWaveform
--------------------------------------------------------------------------------

.. code-block:: cpp

  xmc::audio::Waveform xmc::audio::Tone::getWaveform() const;

現在設定されている出力波形を取得します。

xmc::audio::Tone::setVelocity
--------------------------------------------------------------------------------

.. code-block:: cpp

  void xmc::audio::Tone::setVelocity(uint8_t velo);

ベロシティ (音量) を設定します。値域は 0 から 127 です。
範囲外の値を指定した場合は、この範囲にクリップされます。

xmc::audio::Tone::getVelocity
--------------------------------------------------------------------------------

.. code-block:: cpp

  uint8_t xmc::audio::Tone::getVelocity() const;

現在のベロシティを取得します。

xmc::audio::Tone::setEnvelope
--------------------------------------------------------------------------------

.. code-block:: cpp

  void xmc::audio::Tone::setEnvelope(uint16_t attackMs, uint16_t decayMs, uint16_t sustainLevel, uint16_t releaseMs);

ADSR エンベロープを設定します。

- `attackMs`: アタック時間 [ms]
- `decayMs`: ディケイ時間 [ms]
- `sustainLevel`: サステインレベル (0 から 256)
- `releaseMs`: リリース時間 [ms]

xmc::audio::Tone::setSweep
--------------------------------------------------------------------------------

.. code-block:: cpp

  void xmc::audio::Tone::setSweep(int32_t delta, uint32_t periodMs);

ピッチスイープを設定します。
`periodMs` ごとに現在の周波数に対して係数を乗算し、徐々に音程を変化させます。

`delta` は 16 ビット小数部を持つ固定小数点値として扱われます。
有効範囲は -32768 から 65536 で、内部ではこの範囲にクリップされます。
`delta = 0` の場合はスイープ係数が 1.0 となり、ピッチは変化しません。

xmc::audio::Tone::noteOn
--------------------------------------------------------------------------------

.. code-block:: cpp

  void xmc::audio::Tone::noteOn(uint8_t note, uint32_t lenMs = xmc::audio::TONE_LENGTH_INFINITE);

MIDI ノート番号を指定して再生を開始します。
`note` は 0 から 127 で、69 は A4 (440 Hz) に対応します。

`lenMs` で再生長 [ms] を指定します。
`xmc::audio::TONE_LENGTH_INFINITE` を指定した場合、明示的に停止するまで再生を継続します。

xmc::audio::Tone::noteOnWithFreq
--------------------------------------------------------------------------------

.. code-block:: cpp

  void xmc::audio::Tone::noteOnWithFreq(uint32_t freq, uint32_t lenMs = xmc::audio::TONE_LENGTH_INFINITE);

周波数 [Hz] を直接指定して再生を開始します。
`freq` に 0 を指定した場合は再生を停止します。

xmc::audio::Tone::noteOff
--------------------------------------------------------------------------------

.. code-block:: cpp

  void xmc::audio::Tone::noteOff();

現在のノートを停止します。
`releaseMs` が 0 より大きい場合はリリースフェーズへ遷移し、0 の場合は即座に停止します。

xmc::audio::Tone::mute
--------------------------------------------------------------------------------

.. code-block:: cpp

  void xmc::audio::Tone::mute();

音を即座にミュートします。エンベロープのリリースフェーズは経由しません。

xmc::audio::Tone::render
--------------------------------------------------------------------------------

.. code-block:: cpp

  void xmc::audio::Tone::render(int16_t *buffer, uint32_t numSamples);

指定したバッファに PCM サンプルを加算生成します。
通常はオーディオシステム内部から呼ばれる関数で、アプリケーションコードから直接呼ぶ必要はありません。

xmc::audio::Tone::getOutputPort
--------------------------------------------------------------------------------

.. code-block:: cpp

  xmc::audio::SourcePort *xmc::audio::Tone::getOutputPort();

Tone の出力ポートを取得します。
取得したポートは `xmc::audio::Mixer::setSource()` や `xmc::audio::setSourcePort()` に渡して利用できます。


