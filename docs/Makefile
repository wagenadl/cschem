all:    icons
	sphinx-build -b html source build/html

icons: source/icon-mode-sel.png \
       source/icon-mode-trace.png \
       source/icon-mode-hole.png \
       source/icon-mode-pad.png \
       source/icon-mode-text.png \
       source/icon-mode-arc.png \
       source/icon-mode-plane.png \
       source/icon-mode-nphole.png \
       source/icon-mode-pickup.png \
       source/icon-mode-pnp.png \
       source/icon-mode-angle.png \
       source/icon-para-linewidth.png \
       source/icon-para-diameter.png \
       source/icon-para-slotlength.png \
       source/icon-para-arcstyle.png \
       source/icon-para-orient.png \
       source/icon-action-rotate.png \
       source/icon-toggle-abs.png \
       source/icon-toggle-anchor.png 

source/icon-mode-sel.png: ../cpcb/ui/icons/Edit.svg
	inkscape -o $@ -C $^
source/icon-mode-trace.png: ../cpcb/ui/icons/Trace.svg
	inkscape -o $@ -C $^
source/icon-mode-hole.png: ../cpcb/ui/icons/Hole.svg
	inkscape -o $@ -C $^
source/icon-mode-pad.png: ../cpcb/ui/icons/Pad.svg
	inkscape -o $@ -C $^
source/icon-mode-text.png: ../cpcb/ui/icons/Text.svg
	inkscape -o $@ -C $^
source/icon-mode-arc.png: ../cpcb/ui/icons/Arc.svg
	inkscape -o $@ -C $^
source/icon-mode-plane.png: ../cpcb/ui/icons/Plane.svg
	inkscape -o $@ -C $^
source/icon-mode-nphole.png: ../cpcb/ui/icons/NPHole.svg
	inkscape -o $@ -C $^
source/icon-mode-pickup.png: ../cpcb/ui/icons/Disconnect.svg
	inkscape -o $@ -C $^
source/icon-mode-pnp.png: ../cpcb/ui/icons/PNPOrient.svg
	inkscape -o $@ -C $^
source/icon-mode-angle.png: ../cpcb/ui/icons/NoConstrain45.svg
	inkscape -o $@ -C $^
source/icon-para-linewidth.png: ../cpcb/ui/icons/Width.svg
	inkscape -o $@ -C $^
source/icon-para-diameter.png: ../cpcb/ui/icons/Diameter.svg
	inkscape -o $@ -C $^
source/icon-para-slotlength.png: ../cpcb/ui/icons/SlotLength.svg
	inkscape -o $@ -C $^
source/icon-para-arcstyle.png: ../cpcb/ui/icons/Arc240.svg
	inkscape -o $@ -C $^
source/icon-para-orient.png: ../cpcb/ui/icons/OrientUp.svg
	inkscape -o $@ -C $^
source/icon-action-rotate.png: ../cpcb/ui/icons/RotateCCW.svg
	inkscape -o $@ -C $^
source/icon-toggle-abs.png: ../cpcb/ui/icons/OriginAbs.svg
	inkscape -o $@ -C $^
source/icon-toggle-anchor.png: ../cpcb/ui/icons/AlignHPin.svg
	inkscape -o $@ -C $^

.phony: icons
