// Propertiesbar.cpp

#include "Propertiesbar.h"

class PBData {
public:
  QLabel *xl;
  QDoubleSpinBox *x;
  QLabel *yl;
  QDoubleSpinBox *y;
  QLabel *linewidthl;
  QDoubleSpinBox *linewidth; // for trace
  QLabel *wl; 
  QDoubleSpinBox *w; // for pad
  QLabel *hl;
  QDoubleSpinBox *h; // for pad
  QLabel *idl;
  QDoubleSpinBox *id; // for hole
  QLabel *odl;
  QDoubleSpinBox *od; // for hole
  QLabel *squarel;
  QToolButton *square; // for hole
  QLabel *fsl;
  QDoubleSpinBox *fs; // for text
  QLineEdit *text;
  QToolButton *component; // popup for replacing component
};

  
