/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "spectrograph.h"
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QTimerEvent>

int i = 0;

const int NullTimerId = -1;
const int NullIndex = -1;
const int BarSelectionInterval = 2000;

Spectrograph::Spectrograph(QWidget *parent)
    :   QWidget(parent)
    ,   m_barSelected(NullIndex)
    ,   m_timerId(NullTimerId)
    ,   m_lowFreq(0.0)
    ,   m_highFreq(0.0)
{
    setMinimumHeight(100);
}

Spectrograph::~Spectrograph()
{

}

void Spectrograph::setParams(int numBars, qreal lowFreq, qreal highFreq)
{
    Q_ASSERT(numBars > 0);
    Q_ASSERT(highFreq > lowFreq);
    m_bars.resize(numBars);
    m_lowFreq = lowFreq;
    m_highFreq = highFreq;
    updateBars();
}

void Spectrograph::timerEvent(QTimerEvent *event)
{
    Q_ASSERT(event->timerId() == m_timerId);
    Q_UNUSED(event) // suppress warnings in release builds
    //killTimer(m_timerId);
    m_timerId = NullTimerId;
    m_barSelected = NullIndex;
    update();
}

void Spectrograph::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);

    const int numBars = m_bars.count();

    // Highlight region of selected bar
    if (m_barSelected != NullIndex && numBars) {
        QRect regionRect = rect();
        regionRect.setLeft(m_barSelected * rect().width() / numBars);
        regionRect.setWidth(rect().width() / numBars);

        QColor regionColor(202, 202, 64);

        painter.setBrush(Qt::DiagCrossPattern);
        painter.fillRect(regionRect, regionColor);
        painter.setBrush(Qt::NoBrush);
    }

    QColor barColor(51, 204, 102);
    QColor clipColor(255, 255, 0);

    QColor targetColor(50,50,255); // UTK WELDING
    QColor targetClippedColor(255,100,0); // UTK WELDING

    // Draw the outline
    const QColor gridColor = barColor.darker();
    QPen gridPen(gridColor);
    painter.setPen(gridPen);
    painter.drawLine(rect().topLeft(), rect().topRight());
    painter.drawLine(rect().topRight(), rect().bottomRight());
    painter.drawLine(rect().bottomRight(), rect().bottomLeft());
    painter.drawLine(rect().bottomLeft(), rect().topLeft());

    QVector<qreal> dashes;
    dashes << 2 << 2;
    gridPen.setDashPattern(dashes);
    painter.setPen(gridPen);

    // Draw vertical lines between bars
    if (numBars) {
        const int numHorizontalSections = numBars;
        QLine line(rect().topLeft(), rect().bottomLeft());
        for (int i=1; i<numHorizontalSections; ++i) {
            line.translate(rect().width()/numHorizontalSections, 0);
            painter.drawLine(line);
        }
    }

    // Draw horizontal lines
    const int numVerticalSections = 10;
    QLine line(rect().topLeft(), rect().topRight());
    for (int i=1; i<numVerticalSections; ++i) {
        line.translate(0, rect().height()/numVerticalSections);
        painter.drawLine(line);
    }

    barColor = barColor.lighter();
    barColor.setAlphaF(0.75);
    clipColor.setAlphaF(0.75);
    targetColor.setAlphaF(0.75); // UTK WELDING

    // Draw the bars
    if (numBars) {
        // Calculate width of bars and gaps
        const int widgetWidth = rect().width();
        const int barPlusGapWidth = widgetWidth / numBars;
        const int barWidth = 0.8 * barPlusGapWidth;
        const int gapWidth = barPlusGapWidth - barWidth;
        const int paddingWidth = widgetWidth - numBars * (barWidth + gapWidth);
        const int leftPaddingWidth = (paddingWidth + gapWidth) / 2;
        const int barHeight = rect().height() - 2 * gapWidth;

        int numberOfFrequencyTargets = 0; // UTK WELDING
        int checkedFrequencyTargets[targetFrequencyBars[0]-1];

        while(numberOfFrequencyTargets < targetFrequencyBars[0]-1)
        {//qDebug()<<numberOfFrequencyTargets;
            /* A note on the while loop...
             * This while loop ensures that all of the target frequencies populate.
             * For some reason, this was not happening consistently.
             * The issue is the conditional "check if the current bar is a target bar."
             * Using qDebug, I determined that the values of i and targetFrequencyBars[j]
             * were == when they were supposed to be, but it did not always evaluate as true.
             * I tried to debug this, but this was the only workaround that I could find.
             */

            for (int i=0; i<numBars; ++i) {
                const qreal value = m_bars[i].value;
                Q_ASSERT(value >= 0.0 && value <= 1.0);
                QRect bar = rect();
                bar.setLeft(rect().left() + leftPaddingWidth + (i * (gapWidth + barWidth)));
                bar.setWidth(barWidth);
                bar.setTop(rect().top() + gapWidth + (1.0 - value) * barHeight);
                bar.setBottom(rect().bottom() - gapWidth);

                QColor color = barColor;
                if (m_bars[i].clipped)
                    color = clipColor;

                // -- UTK WELDING -- //
                bool isTargetFrequency = false;
                int targetFrequencyIndex;

                for(int j = 1; j < targetFrequencyBars[0]; j++)
                {
                    // Check if the current bar is a target bar
                    if (i == targetFrequencyBars[j] && targetFrequencyBars[j] != 0)
                    {
                        isTargetFrequency = true;
                        targetFrequencyIndex = j;

                        checkedFrequencyTargets[j-1] = targetFrequencyBars[j];

                        if(checkedFrequencyTargets[j-1])
                            numberOfFrequencyTargets++;
                    }
                }

                if(isTargetFrequency)
                {//qDebug()<<m_bars[i].value;
                    color = targetColor;

                    double temp = maxIntensityLevel[targetFrequencyIndex];
                    double currentValue = m_bars[i].value;
                    double intensityThreshold = temp/100;

                    if(currentValue >= intensityThreshold)
                    {
                        color = targetClippedColor;
                        emit targetFrequencyIsClipped(targetFrequencyIndex);
                    }//if
                }//if

                painter.fillRect(bar, color);

                if(isTargetFrequency)
                {
                    QRect regionRect = rect();
                    regionRect.setLeft(i * rect().width() / numBars);
                    regionRect.setWidth(rect().width() / numBars);

                    QColor regionColor(150, 150, 150);
                    regionColor.setAlphaF(0.3);
                    painter.fillRect(regionRect, regionColor);

                    // --- max intensity clip lines --- //
                    //configure pen
                    QColor intensityColor(255, 25, 25);
                    QPen intensityPen(intensityColor);
                    painter.setPen(intensityPen);

                    //create line
                    QPoint left(i*rect().width()/numBars,0);
                    QPoint right((i+1)*rect().width()/numBars,0);
                    QLine intensity(left,right);

                    //move and draw line
                    intensity.translate(0,rect().height()*(100-maxIntensityLevel[targetFrequencyIndex])/100);
                    painter.drawLine(intensity);

                    //reset pen
                    painter.setPen(gridPen);
                }//if
                // ~UTK WELDING
            }//for i

        }//while

        if(numberOfFrequencyTargets != targetFrequencyBars[0]-1)
        {
            qDebug()<<"Missing Frequency Target(s)!";
        }

    }//if numbars
}

void Spectrograph::mousePressEvent(QMouseEvent *event)
{
    const QPoint pos = event->pos();
    const int index = m_bars.count() * (pos.x() - rect().left()) / rect().width();
    selectBar(index);
}

void Spectrograph::reset()
{
    m_spectrum.reset();
    spectrumChanged(m_spectrum);
}

void Spectrograph::spectrumChanged(const FrequencySpectrum &spectrum)
{
    m_spectrum = spectrum;
    updateBars();
}

int Spectrograph::barIndex(qreal frequency) const
{
    Q_ASSERT(frequency >= m_lowFreq && frequency < m_highFreq);
    const qreal bandWidth = (m_highFreq - m_lowFreq) / m_bars.count();
    const int index = (frequency - m_lowFreq) / bandWidth;
    if (index <0 || index >= m_bars.count())
        Q_ASSERT(false);
    return index;
}

QPair<qreal, qreal> Spectrograph::barRange(int index) const
{
    Q_ASSERT(index >= 0 && index < m_bars.count());
    const qreal bandWidth = (m_highFreq - m_lowFreq) / m_bars.count();
    return QPair<qreal, qreal>(index * bandWidth, (index+1) * bandWidth);
}

void Spectrograph::updateBars()
{
    m_bars.fill(Bar());
    FrequencySpectrum::const_iterator i = m_spectrum.begin();
    const FrequencySpectrum::const_iterator end = m_spectrum.end();
    for ( ; i != end; ++i) {
        const FrequencySpectrum::Element e = *i;
        if (e.frequency >= m_lowFreq && e.frequency < m_highFreq) {
            Bar &bar = m_bars[barIndex(e.frequency)];
            bar.value = qMax(bar.value, e.amplitude);
            bar.clipped |= e.clipped;
        }
    }
    update();
}

void Spectrograph::selectBar(int index) {
    const QPair<qreal, qreal> frequencyRange = barRange(index);
    const QString message = QString("%1 - %2 Hz")
                                .arg(frequencyRange.first)
                                .arg(frequencyRange.second);
    emit infoMessage(message, BarSelectionInterval);

    //if (NullTimerId != m_timerId)
        //killTimer(m_timerId);
    //m_timerId = startTimer(BarSelectionInterval);

    m_barSelected = index;
    update();
}

// ----- UTK WELDING ----- //
void Spectrograph::setTargetFrequencies(int *frequencies, int* maximumIntensity)
{
    if(frequencies[0] != maximumIntensity[0])
    {
        qDebug()<<"Frequency and Intensity vector dimension mismatch!";
        return;
    }

    for(int i = 0; i <= maximumIntensity[0]; i++)
    {
        maxIntensityLevel[i] = maximumIntensity[i];
    }

    int numBars = m_bars.count();
    double increment = (SpectrumHighFreq - SpectrumLowFreq)/(numBars);

    targetFrequencyBars[0] = frequencies[0];

    QString debugFreqBarList = "Bars:";
    for(int i = 1; i <= frequencies[0] - 1; i++) // for each target frequency
    {
        debugFreqBarList = debugFreqBarList  + " ";
        for(int j = 1; j < numBars; j++) // for each division of the graph
        {
            if (frequencies[i] > increment * j && frequencies[i] <= SpectrumHighFreq)
            {
                targetFrequencyBars[i] = j;
            }
        }//for j
        debugFreqBarList = debugFreqBarList + QString::number(targetFrequencyBars[i]);
    }//for i
    qDebug()<<debugFreqBarList;
}
