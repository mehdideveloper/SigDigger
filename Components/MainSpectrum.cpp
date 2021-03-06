//
//    MainSpectrum.cpp: Main spectrum component
//    Copyright (C) 2019 Gonzalo José Carracedo Carballal
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as
//    published by the Free Software Foundation, either version 3 of the
//    License, or (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful, but
//    WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this program.  If not, see
//    <http://www.gnu.org/licenses/>
//

#include "MainSpectrum.h"
#include "ui_MainSpectrum.h"

using namespace SigDigger;

MainSpectrum::MainSpectrum(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::MainSpectrum)
{
  ui->setupUi(this);
  this->connectAll();
  this->setCenterFreq(0);
}

MainSpectrum::~MainSpectrum()
{
  delete ui;
}

void
MainSpectrum::connectAll(void)
{
  connect(
        this->ui->mainSpectrum,
        SIGNAL(newFilterFreq(int, int)),
        this,
        SLOT(onWfBandwidthChanged(int, int)));

  connect(
        this->ui->fcLcd,
        SIGNAL(valueChanged(void)),
        this,
        SLOT(onFrequencyChanged(void)));

  connect(
        this->ui->lnbLcd,
        SIGNAL(valueChanged(void)),
        this,
        SLOT(onLnbFrequencyChanged(void)));

  connect(
        this->ui->loLcd,
        SIGNAL(valueChanged(void)),
        this,
        SLOT(onLoChanged(void)));

  connect(
        this->ui->mainSpectrum,
        SIGNAL(newDemodFreq(qint64, qint64)),
        this,
        SLOT(onWfLoChanged(void)));

  connect(
        this->ui->mainSpectrum,
        SIGNAL(pandapterRangeChanged(float, float)),
        this,
        SLOT(onRangeChanged(float, float)));

  connect(
        this->ui->mainSpectrum,
        SIGNAL(newZoomLevel(float)),
        this,
        SLOT(onNewZoomLevel(float)));
}

void
MainSpectrum::feed(float *data, int size)
{
  this->ui->mainSpectrum->setNewFftData(data, size);
}

void
MainSpectrum::refreshUi(void)
{
  QString modeText = "  Capture mode: ";

  switch (this->mode) {
    case UNAVAILABLE:
      modeText += "N/A";
      break;

    case CAPTURE:
      modeText += "LIVE";
      break;

    case REPLAY:
      modeText += "REPLAY";
      break;
  }

  this->ui->captureModeLabel->setText(modeText);

  if (this->throttling)
    this->ui->throttlingLabel->setText("  Throttling: ON");
  else
    this->ui->throttlingLabel->setText("  Throttling: OFF");
}

void
MainSpectrum::setThrottling(bool value)
{
  this->throttling = value;
  this->refreshUi();
}

void
MainSpectrum::setTimeSpan(quint64 span)
{
  this->ui->mainSpectrum->setWaterfallSpan(span * 1000);
}

void
MainSpectrum::setCaptureMode(CaptureMode mode)
{
  this->mode = mode;
  this->refreshUi();
}

int
MainSpectrum::getFrequencyUnits(qint64 freq)
{
  if (freq < 0)
    freq = -freq;

  if (freq < 1000)
    return 1;

  if (freq < 1000000)
    return 1000;

  if (freq < 1000000000)
    return 1000000;

  return 1000000000;
}

void
MainSpectrum::setCenterFreq(qint64 freq)
{
  this->ui->fcLcd->setValue(freq);
  this->ui->mainSpectrum->setCenterFreq(static_cast<quint64>(freq));
  this->ui->mainSpectrum->setFreqUnits(
        getFrequencyUnits(
          static_cast<qint64>(freq)));
}

void
MainSpectrum::setLoFreq(qint64 loFreq)
{
  this->ui->loLcd->setValue(loFreq);
  this->ui->mainSpectrum->setFilterOffset(loFreq);
}

void
MainSpectrum::setLnbFreq(qint64 lnbFreq)
{
  this->ui->lnbLcd->setValue(lnbFreq);
  this->ui->fcLcd->setMin(-lnbFreq);
}

void
MainSpectrum::setPaletteGradient(const QColor *table)
{
  this->ui->mainSpectrum->setPalette(table);
}

void
MainSpectrum::setPandapterRange(float min, float max)
{
  this->ui->mainSpectrum->setPandapterRange(min, max);
}

void
MainSpectrum::setWfRange(float min, float max)
{
  this->ui->mainSpectrum->setWaterfallRange(min, max);
}

void
MainSpectrum::setPanWfRatio(float ratio)
{
  this->ui->mainSpectrum->setPercent2DScreen(static_cast<int>(ratio * 100));
}

void
MainSpectrum::setPeakHold(bool hold)
{
  this->ui->mainSpectrum->setPeakHold(hold);
}

void
MainSpectrum::setPeakDetect(bool det)
{
  this->ui->mainSpectrum->setPeakDetection(det, 5);
}


void
MainSpectrum::setColorConfig(ColorConfig const &cfg)
{
  QString styleSheet =
      "background-color: " + cfg.lcdBackground.name() + "; \
      color: " + cfg.lcdForeground.name() + "; \
      font-size: 12px; \
      font-family: Monospace; \
      font-weight: bold;";

  this->ui->fcLcd->setForegroundColor(cfg.lcdForeground);
  this->ui->fcLcd->setBackgroundColor(cfg.lcdBackground);
  this->ui->loLcd->setForegroundColor(cfg.lcdForeground);
  this->ui->loLcd->setBackgroundColor(cfg.lcdBackground);
  this->ui->lnbLcd->setForegroundColor(cfg.lcdForeground);
  this->ui->lnbLcd->setBackgroundColor(cfg.lcdBackground);

  this->ui->loLabel->setStyleSheet(styleSheet);
  this->ui->lnbLabel->setStyleSheet(styleSheet);
  this->ui->captureModeLabel->setStyleSheet(styleSheet);
  this->ui->throttlingLabel->setStyleSheet(styleSheet);

  this->ui->mainSpectrum->setFftPlotColor(cfg.spectrumForeground);
  this->ui->mainSpectrum->setFftAxesColor(cfg.spectrumAxes);
  this->ui->mainSpectrum->setFftBgColor(cfg.spectrumBackground);
  this->ui->mainSpectrum->setFftTextColor(cfg.spectrumText);
}

void
MainSpectrum::setFilterBandwidth(unsigned int bw)
{
  if (this->bandwidth != bw) {
    int freq = static_cast<int>(bw);
    this->ui->mainSpectrum->setHiLowCutFrequencies(
          -freq / 2,
          freq/2);
    this->bandwidth = bw;
  }
}

void
MainSpectrum::setZoom(unsigned int zoom)
{
  if (zoom > 0) {
    this->zoom = zoom;
    this->ui->mainSpectrum->setSpanFreq(
          this->cachedRate / zoom);
  }
}

void
MainSpectrum::setSampleRate(unsigned int rate)
{
  if (this->cachedRate != rate) {
    int freq = static_cast<int>(rate);

    this->ui->mainSpectrum->setDemodRanges(-freq / 2, 1, 1, freq / 2, true);

    this->ui->mainSpectrum->setSampleRate(rate);

    this->ui->mainSpectrum->setSpanFreq(rate / this->zoom);
    this->ui->loLcd->setMin(-freq / 2);
    this->ui->loLcd->setMax(freq / 2);

    this->cachedRate = rate;
  }
}

////////////////////////////////// Getters ////////////////////////////////////
bool
MainSpectrum::getThrottling(void) const
{
  return this->throttling;
}

MainSpectrum::CaptureMode
MainSpectrum::getCaptureMode(void) const
{
  return this->mode;
}

qint64
MainSpectrum::getCenterFreq(void) const
{
  return this->ui->fcLcd->getValue();
}

qint64
MainSpectrum::getLoFreq(void) const
{
  return this->ui->loLcd->getValue();
}

qint64
MainSpectrum::getLnbFreq(void) const
{
  return this->ui->lnbLcd->getValue();
}

unsigned int
MainSpectrum::getBandwidth(void) const
{
  return this->bandwidth;
}

//////////////////////////////// Slots /////////////////////////////////////////
void
MainSpectrum::onWfBandwidthChanged(int min, int max)
{
  this->setFilterBandwidth(static_cast<unsigned int>(max - min));
  emit bandwidthChanged();
}

void
MainSpectrum::onFrequencyChanged(void)
{
  qint64 freq = this->ui->fcLcd->getValue();
  this->setCenterFreq(freq);
  emit frequencyChanged(freq);
}

void
MainSpectrum::onLnbFrequencyChanged(void)
{
  qint64 freq = this->ui->lnbLcd->getValue();
  this->setLnbFreq(freq);
  emit lnbFrequencyChanged(freq);
}

void
MainSpectrum::onWfLoChanged(void)
{
  this->ui->loLcd->setValue(this->ui->mainSpectrum->getFilterOffset());
  emit loChanged(this->getLoFreq());
}

void
MainSpectrum::onLoChanged(void)
{
  this->ui->mainSpectrum->setFilterOffset(this->ui->loLcd->getValue());
  emit loChanged(this->getLoFreq());
}

void
MainSpectrum::onRangeChanged(float min, float max)
{
  emit rangeChanged(min, max);
}

void
MainSpectrum::onNewZoomLevel(float level)
{
  emit zoomChanged(level);
}
