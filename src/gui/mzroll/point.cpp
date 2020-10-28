#include "common/analytics.h"
#include "Compound.h"
#include "eiclogic.h"
#include "eicwidget.h"
#include "globals.h"
#include "grouprtwidget.h"
#include "isotopeplot.h"
#include "isotopeswidget.h"
#include "ligandwidget.h"
#include "mainwindow.h"
#include "masscalcgui.h"
#include "mzSample.h"
#include "point.h"
#include "Scan.h"
#include "spectrawidget.h"
#include "treedockwidget.h"

EicPoint::EicPoint(float x, float y, Peak* peak, MainWindow* mw)
{

    setFlag(QGraphicsItem::ItemIsFocusable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);

    _x = x;
    _y = y;
    _mw = mw;
    _peak = peak;
    _scan = nullptr;
    _group = nullptr;
    _cSize = 10;
    _color=QColor(Qt::black);
    _pen=QPen(_color);
    _brush=QBrush(_color);

    setPointShape(CIRCLE);
    forceFillColor(false);

    if (_peak) {
        _cSize += 20*(_peak->quality);
        // mouse press events
        connect(this,
                &EicPoint::peakSelected,
                mw,
                &MainWindow::showPeakInfo);
        connect(this,
                &EicPoint::peakSelected,
                mw->getEicWidget(),
                &EicWidget::showPeakArea);
        connect(this,
                &EicPoint::peakSelected,
                mw->spectraWidget,
                QOverload<Peak*>::of(&SpectraWidget::setScan));
        connect(this,
                &EicPoint::peakGroupSelected,
                mw->fragSpectraWidget,
                &SpectraWidget::overlayPeakGroup);

        // mouse hover events
        connect(this,
                &EicPoint::peakGroupFocus,
                mw->getEicWidget(),
                &EicWidget::setSelectedGroup);
        connect(this,
                &EicPoint::peakGroupFocus,
                [this] {
                    _mw->getEicWidget()->scene()->update();
                });
    }
    connect(this,
            &EicPoint::peakGroupSelected,
            [this] (shared_ptr<PeakGroup> group) {
                _mw->massCalcWidget->setPeakGroup(group.get());
            });
    connect(this,
            &EicPoint::ms2MarkerSelected,
            mw->massCalcWidget,
            &MassCalcWidget::setFragmentationScan);
}

EicPoint::~EicPoint() {}

void EicPoint::removeFromScene()
{
    prepareGeometryChange();
    if (scene() != nullptr)
        scene()->removeItem(this);
}

QRectF EicPoint::boundingRect() const
{
    return(QRectF(_x-_cSize/2,_y-_cSize/2,_cSize,_cSize));
}

void EicPoint::hoverEnterEvent (QGraphicsSceneHoverEvent*) {
	this->setFocus(Qt::MouseFocusReason);

	//update colors of all peaks belonging to this group
    if(_group != nullptr) {
		Q_FOREACH (QGraphicsItem *item, scene()->items()) {
            auto point = qgraphicsitem_cast<EicPoint*>(item);
            if (point && point->getPeakGroup() == _group)
                item->update();
		}
	}

	string sampleName;
    if (_peak && _peak->getSample() ) {
        auto sample = _peak->getSample();
        QString sampleNumber =
            sample->sampleNumber != -1 ? QString::number(sample->sampleNumber)
                                       : "NA";
        sampleName = _peak->getSample()->sampleName;

        auto quantType = _mw->getUserQuantType();
        float quantity = _peak->peakIntensity;
        switch (quantType) {
            case PeakGroup::Area:
                quantity = _peak->peakAreaCorrected;
                break;
            case PeakGroup::AreaTop:
                quantity = _peak->peakAreaTopCorrected;
                break;
            case PeakGroup::AreaNotCorrected:
                quantity = _peak->peakArea;
                break;
            case PeakGroup::AreaTopNotCorrected:
                quantity = _peak->peakAreaTop;
                break;
            case PeakGroup::Quality:
                quantity = _peak->quality;
                break;
            case PeakGroup::RetentionTime:
                quantity = _peak->rt;
            default:
                break;
        }

        setToolTip(
            QString("<b> Sample: </b>%1").arg(sampleName.c_str())
            + QString("<br> <b>%1: </b>%2")
                  .arg(_mw->quantType->currentText())
                  .arg(QString::number(quantity))
            + QString("<br> <b>Corrected area: </b>%1")
                  .arg(QString::number(_peak->peakAreaCorrected))
            + QString("<br> <b>Spline Area: </b>%1")
                  .arg(QString::number(_peak->peakSplineArea))
            + QString("<br> <b>Width: </b>%1")
                  .arg(QString::number(_peak->width))
            + QString("<br> <b>m/z: </b>%1")
                  .arg(QString::number(_peak->peakMz, 'f', 6))
            + QString("<br> <b>RT: </b>%1")
                  .arg(QString::number(_peak->rt, 'f', 2))
            + QString("<br> <b>Scan number: </b>%1")
                  .arg(QString::number(_peak->scan))
            + QString("<br> <b>Sample number: </b>%1").arg(sampleNumber));

		update();
		/*
		   "<br> <b>quality:  </b>"  + QString::number(_peak->quality, 'f', 2) +
		   "<br> <b>sigma:  </b>"  + QString::number(_peak->gaussFitSigma, 'f', 2) +
		   "<br> <b>fitR2:  </b>"  + QString::number(_peak->gaussFitR2*100, 'f', 2)
		   "<br> <b>Group Overlap Frac: </b>" + QString::number(_peak->groupOverlapFrac)
		   "<br> <b>peakAreaFractional: </b>" + QString::number(_peak->peakAreaFractional) +
		   "<br> <b>noNoiseFraction: </b>" + QString::number(_peak->noNoiseFraction) +
		   "<br> <b>symmetry: </b>" + QString::number(_peak->symmetry) +
		   "<br> <b>sigma: </b>" + QString::number(_peak->gaussFitSigma) +
		   "<br> <b>r2: </b>" + QString::number(_peak->gaussFitR2) +
		   "<br> <b>angle: </b>" + QString::number(_peak->angle) +
		   "<br> <b>rank: </b>" + QString::number(_peak->peakRank) +
		   "<br> <b>S/N: </b>" + QString::number(_peak->signalBaselineRatio, 'f', 4) +
		   "<br> <b>Width: </b>" + QString::number(_peak->width) +
		   "<br> <b>No NoiseObs: </b>" + QString::number(_peak->noNoiseObs) +
		   "<br> <b>Group Overlap Frac: </b>" + QString::number(_peak->groupOverlapFrac) +
		 */
	} 
    if (_scan) {
        auto sample = _scan->getSample();
        QString sampleNumber =
            sample->sampleNumber != -1 ? QString::number(sample->sampleNumber)
                                       : "NA";
        setToolTip(
            QString("<b> Sample: </b>%1")
                .arg(_scan->sample->sampleName.c_str())
            + QString("<br> <b>Filterline: </b>%1")
                  .arg(_scan->filterLine.c_str())
            + QString("<br> <b>Scan number: </b>%1")
                  .arg(QString::number(_scan->scannum))
            + QString("<br> <b>Sample number: </b>%1")
                  .arg(sampleNumber)
            + QString("<br> <b>Precursor m/z: </b>%1")
                  .arg(QString::number(_scan->precursorMz, 'f', 2)));
	}

    if(_group != nullptr) {
        _group->isFocused = true;
        Q_EMIT(peakGroupFocus(_group));
    }
}

void EicPoint::hoverLeaveEvent ( QGraphicsSceneHoverEvent*) {
    clearFocus();

    if (_group != nullptr) {
        _group->isFocused = false;

        Q_FOREACH (QGraphicsItem *item, scene()->items()) {
            auto point = qgraphicsitem_cast<EicPoint*>(item);
            if (point && point->getPeakGroup() == _group)
                point->update();
        }
    }
    update(); 
}

void EicPoint::mouseDoubleClickEvent(QGraphicsSceneMouseEvent*) {

    if (_group != nullptr) Q_EMIT peakGroupSelected(_group);
    if (_peak)  Q_EMIT peakSelected(_peak);

    if (_group != nullptr) {
        _group = make_shared<PeakGroup>(*_group,
                                        PeakGroup::IntegrationType::Manual);
        auto slice = _group->getSlice();
        auto bounds = _mw->getEicWidget()->visibleSamplesBounds();
        slice.rtmin = bounds.rtmin;
        slice.rtmax = bounds.rtmax;
        _group->setSlice(slice);
        _mw->isotopeWidget->setPeakGroupAndMore(_group, true);
    }

}

void EicPoint::mousePressEvent (QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::RightButton)  {
        contextMenuEvent(event);
        return;
    }

    setZValue(10);
    if (_group != nullptr)
        emit peakGroupSelected(_group);

    if (_peak) {
        emit peakSelected(_peak);
    } else {
        emit ms2MarkerSelected(_scan);
    }

    // make changes through static functions, since this object might be
    // destroyed during the execution period of those functions.
    EicPoint::_updateWidgetsForScan(_mw, _scan);
    EicPoint::_updateWidgetsForPeakGroup(_mw, _group, _peak);
}

void EicPoint::_updateWidgetsForPeakGroup(MainWindow* mw,
                                          shared_ptr<PeakGroup> group,
                                          Peak* peak)
{
    if (mw == nullptr)
        return;

    if (group != nullptr)
        mw->groupRtWidget->plotGraph(group.get());
    if (peak != nullptr
        && group != nullptr
        && (mw->isotopeWidget->isVisible() || mw->isotopePlot->isVisible())) {
        mw->isotopeWidget->peakSelected(peak, group);
    }
    if (peak && mw->covariantsPanel->isVisible())
        mw->getLinks(peak);
    if (peak == nullptr)
        //ms2 markers have no peaks
        mw->getAnalytics()->hitEvent("DDA", "ClickedOnMarker");
}

void EicPoint::_updateWidgetsForScan(MainWindow* mw, Scan* scan)
{
    if (scan != nullptr && mw != nullptr) {
        if (mw->spectraWidget)
            mw->spectraWidget->setScan(scan);
        if (mw->fragSpectraWidget) {
            mw->fragSpectraDockWidget->setVisible(true);
            mw->fragSpectraDockWidget->raise();
            mw->fragSpectraWidget->overlayScan(scan);
        }
    }
}

void EicPoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QPen pen = _pen;
    QBrush brush = _brush;
    painter->setRenderHint(QPainter::Antialiasing);

    float scale = min(scene()->height(),scene()->width())/500;
    float paintDiameter = _cSize*scale;  
    if (paintDiameter<5) paintDiameter=5; if (paintDiameter>30) paintDiameter=30;

    //int maxRadius = scene()->height()*0.05;
    //if (maxRadius > 30) maxRadius = 30;
    //if (maxRadius < 5) maxRadius=5;

    shared_ptr<PeakGroup> selGroup =
        _mw->getEicWidget()->getParameters()->displayedGroup();

    if (_group != nullptr && selGroup == _group) {
        brush.setStyle(Qt::SolidPattern);
        pen.setColor(_color.darker());
        pen.setWidth(_pen.width());
    } else {
        brush.setStyle(Qt::NoBrush);
    }

    if (_forceFill) {
        brush.setStyle(Qt::SolidPattern);
    }

    painter->setPen(pen);
    painter->setBrush(brush);
    //float cSize = 6 + maxRadius*(_peak->quality);
    //painter->drawEllipse(_x-cSize/2, _y-cSize/2, cSize,cSize);

    if(pointShape == CIRCLE ) {
        painter->drawEllipse(_x-paintDiameter/2, _y-paintDiameter/2, paintDiameter,paintDiameter);
    } else if (pointShape == SQUARE) {
        painter->drawRect(_x-paintDiameter/2, _y-paintDiameter/2, paintDiameter,paintDiameter);
    } else if (pointShape == TRIANGLE_DOWN ) {
        painter->drawPie(_x-paintDiameter/2,_y-paintDiameter/2,paintDiameter,paintDiameter,30*16,120*16);
    } else if (pointShape == TRIANGLE_UP ) {
        painter->drawPie(_x-paintDiameter/2,_y-paintDiameter/2,paintDiameter,paintDiameter,260*16,20*16);
    }
}

void EicPoint::setClipboardToGroup()
{
    _mw->getAnalytics()->hitEvent("Exports", "Clipboard", "Group Information");
    if(_group != nullptr)
        _mw->setClipboardToGroup(_group.get());
}

void EicPoint::bookmark()
{
    if(_group != nullptr)
        _mw->bookmarkPeakGroup(_group);
}

void EicPoint::setClipboardToIsotopes() {
    _mw->getAnalytics()->hitEvent("Exports",
                                  "Clipboard",
                                  "Isotopes Information");
    if (_group != nullptr
        && _group->getCompound() != NULL
        && ! _group->getCompound()->formula().empty()) {
        _mw->isotopeWidget->setPeakGroupAndMore(_group, true);
        if (_peak)
            _mw->isotopeWidget->peakSelected(_peak, _group);
    }
}

void EicPoint::linkCompound() {
    if (_group != nullptr &&_group->getCompound() != NULL )  {

            //update compound retention time
            if (_peak) _group->getCompound()->setExpectedRt(_peak->rt);

            //log information about retention time change
           // _mw->getEicWidget()->addNote(_peak->peakMz,_peak->peakIntensity, "Compound Link");
            _mw->getEicWidget()->saveRetentionTime();

            //_mw->ligandWidget->updateTable();
            _mw->ligandWidget->updateCurrentItemData();
	}
}

void EicPoint::reorderSamples()
{
    if (_mw && _group != nullptr)
        _mw->reorderSamples(_group.get());
}

void EicPoint::contextMenuEvent ( QGraphicsSceneMouseEvent* event ) {
    QMenu menu;

    QAction* c1 = menu.addAction("Copy Details to Clipboard");
    c1->setIcon(QIcon(rsrcPath + "/copyCSV.png"));
    connect(c1, SIGNAL(triggered()), SLOT(setClipboardToGroup()));

    if (_group != nullptr && _group->getCompound() ) {
       if ( _group->isIsotope() == false
            && !_group->getCompound()->formula().empty() ) {
            QAction* z = menu.addAction("Copy Isotope Information to Clipboard");
            z->setIcon(QIcon(rsrcPath + "/copyCSV.png"));
            connect(z, SIGNAL(triggered()), SLOT(setClipboardToIsotopes()));
        }

        QAction* e = menu.addAction("Link to Compound");
        e->setIcon(QIcon(rsrcPath + "/link.png"));
        connect(e, SIGNAL(triggered()), SLOT(linkCompound()));
    }

    QAction* c2 = menu.addAction("Mark Good");
    c2->setIcon(QIcon(rsrcPath + "/markgood.png"));
    connect(c2, SIGNAL(triggered()), _mw->getEicWidget(), SLOT(markGroupGood()));

    QAction* c3 = menu.addAction("Mark Bad");
    c3->setIcon(QIcon(rsrcPath + "/markbad.png"));
    connect(c3, SIGNAL(triggered()), _mw->getEicWidget(), SLOT(markGroupBad()));

    if ( _group != nullptr && _group->peaks.size() > 1  ) {
        QAction* d = menu.addAction("Sort Samples by Peak Intensity");
        connect(d, SIGNAL(triggered()), SLOT(reorderSamples()));
    }

    QAction *selectedAction = menu.exec(event->screenPos());


    //event->ignore();
}

void EicPoint::keyPressEvent( QKeyEvent *e ) {
	bool marked=false;
    if (_group == nullptr) return;

	//qDebug() << "Point::keyPressEvent() " << e->key() << endl;

	switch( e->key() ) {
	}
	update();
	e->accept();
}
