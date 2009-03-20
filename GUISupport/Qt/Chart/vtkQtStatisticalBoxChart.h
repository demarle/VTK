/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkQtStatisticalBoxChart.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/

/// \file vtkQtStatisticalBoxChart.h
/// \date May 15, 2008

#ifndef _vtkQtStatisticalBoxChart_h
#define _vtkQtStatisticalBoxChart_h

#include "vtkQtChartExport.h"
#include "vtkQtChartSeriesLayer.h"

class vtkQtStatisticalBoxChartInternal;
class vtkQtStatisticalBoxChartOptions;
class vtkQtStatisticalBoxChartSeriesOptions;


/// \class vtkQtStatisticalBoxChart
/// \brief
///   The vtkQtStatisticalBoxChart class is used to display a statistical box chart.
class VTKQTCHART_EXPORT vtkQtStatisticalBoxChart : public vtkQtChartSeriesLayer
{
  Q_OBJECT

public:
  vtkQtStatisticalBoxChart();
  virtual ~vtkQtStatisticalBoxChart();

  /// \name Setup Methods
  //@{
  virtual void setChartArea(vtkQtChartArea *area);

  virtual void setModel(vtkQtChartSeriesModel *model);
  //@}

  /// \name Drawing Parameters
  //@{
  /// \brief
  ///   Gets the box chart drawing options.
  /// \return
  ///   A pointer to the box chart drawing options.
  vtkQtStatisticalBoxChartOptions *getOptions() const {return this->Options;}

  /// \brief
  ///   Sets the box chart drawing options.
  ///
  /// This method sets all the options at once, which can prevent
  /// unnecessary view updates.
  ///
  /// \param options The new box chart drawing options.
  void setOptions(const vtkQtStatisticalBoxChartOptions &options);

  /// \brief
  ///   Gets the statistical box chart series options.
  /// \param series The series index.
  /// \return
  ///   A pointer to the statistical box chart series options.
  vtkQtStatisticalBoxChartSeriesOptions *getBoxSeriesOptions(int series) const;

  virtual QPixmap getSeriesIcon(int series) const;
  //@}

  /// \name Layout Methods
  //@{
  virtual void getLayerDomain(vtkQtChartLayerDomain &domain) const;

  virtual void layoutChart(const QRectF &area);

  virtual bool getHelpText(const QPointF &point, QString &text);

  /// \brief
  ///   Notifies the chart layer that a resize interaction has finished.
  ///
  /// The chart search tree is not updated while the chart is in an
  /// interactive state. It is updated in this method if needed.
  virtual void finishInteractiveResize();
  //@}

  /// \name Selection Methods
  //@{
  virtual void getSeriesAt(const QPointF &point,
      vtkQtChartSeriesSelection &selection) const;

  virtual void getPointsAt(const QPointF &point,
      vtkQtChartSeriesSelection &selection) const;

  virtual void getSeriesIn(const QRectF &area,
      vtkQtChartSeriesSelection &selection) const;

  virtual void getPointsIn(const QRectF &area,
      vtkQtChartSeriesSelection &selection) const;
  //@}

  /// \name QGraphicsItem Methods
  //@{
  virtual QRectF boundingRect() const;

  virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
      QWidget *widget=0);
  //@}

public slots:
  /// \brief
  ///   Refreshes the statistical box chart data from the model.
  ///
  /// The currently displayed data is cleaned up. If a model is set,
  /// it is used to populate the statistical box chart.
  void reset();

protected:
  /// \brief
  ///   Creates a new statistical box chart series options object.
  /// \param parent The parent object.
  /// \return
  ///   A pointer to the new statistical box chart series options object.
  virtual vtkQtChartSeriesOptions *createOptions(QObject *parent);

  /// \brief
  ///   Sets up the series options defaults.
  ///
  /// The style manager's "Visible" generator is used to set the
  /// initial visibility. The style manager's "Brush" generator is used
  /// to set the series brush. The series pen is set to black or a darker
  /// version of the series brush color. The "Marker Style" and
  /// "Marker Size" generators are used to set up the outlier points.
  ///
  /// \param style The series style index.
  /// \param options The new series options object.
  virtual void setupOptions(int style, vtkQtChartSeriesOptions *options);

private slots:
  /// \brief
  ///   Prepares the statistical box chart for a series insertion.
  /// \param first The first series index to be added.
  /// \param last The last series index to be added.
  void prepareSeriesInsert(int first, int last);

  /// \brief
  ///   Inserts statistical box chart series at the given indexes.
  /// \param first The first series index to be added.
  /// \param last The last series index to be added.
  void insertSeries(int first, int last);

  /// \brief
  ///   Removes the specified series from the chart.
  /// \param first The first series index to be removed.
  /// \param last The last series index to be removed.
  void startSeriesRemoval(int first, int last);

  /// \brief
  ///   Finishes the domain changes after removing the series.
  /// \param first The first series index to be removed.
  /// \param last The last series index to be removed.
  void finishSeriesRemoval(int first, int last);

  /// Requests a chart layout when the axis corner option changes.
  void handleAxesCornerChange();

  /// Changes the box outline style.
  void handleOutlineChange();

  /// \brief
  ///   Changes the series visibility.
  ///
  /// The signal sender is used to determine which series has changed.
  ///
  /// \param visible True if the series should be shown.
  void handleSeriesVisibilityChange(bool visible);

  /// \brief
  ///   Changes the series pen.
  /// \param pen The new series pen.
  void handleSeriesPenChange(const QPen &pen);

  /// \brief
  ///   Changes the series brush.
  /// \param brush The new series brush.
  void handleSeriesBrushChange(const QBrush &brush);

  /// Changes the series point marker.
  void handleSeriesPointMarkerChanged();

  /// \brief
  ///   Called to set up the highlights.
  ///
  /// The set up request is ignored if the model is being changed.
  void updateHighlights();

private:
  /// \brief
  ///   Adds the domain for the given series to the current domain.
  /// \param series The series index.
  /// \param seriesGroup Used to return the series group index.
  /// \return
  ///   True if the domain was modified.
  bool addSeriesDomain(int series, int &seriesGroup);

  /// \brief
  ///   Calculates the domain for the given series group.
  /// \param seriesGroup The series group index.
  void calculateDomain(int seriesGroup);

  /// \brief
  ///   Creates an ordered table of series shapes.
  /// \param seriesGroup The series group index.
  void createShapeTable(int seriesGroup);

  /// \brief
  ///   Builds the shape tree for the given series group.
  /// \param seriesGroup The series group index.
  void buildShapeTree(int seriesGroup);

private:
  vtkQtStatisticalBoxChartInternal *Internal; ///< Stores the box series.
  vtkQtStatisticalBoxChartOptions *Options;   ///< Stores the drawing options.
  bool InModelChange;                         ///< Used for selection changes.
  bool BuildNeeded;                           ///< Used for interactive resize.

private:
  vtkQtStatisticalBoxChart(const vtkQtStatisticalBoxChart &);
  vtkQtStatisticalBoxChart &operator=(const vtkQtStatisticalBoxChart &);
};

#endif
