HHC_BINARY := $(shell `which hhc` --help 2>/dev/null)
APP_NAME := sensor_hub

all: html info pdf chm


html: 
	mkdir -p /cygdrive/c/Sandbox/damen_sensor_hub/manual/manual/html/images
	mkdir -p /cygdrive/c/Sandbox/damen_sensor_hub/manual/manual/html/styles
	/cygdrive/c/Sandbox/damen_sensor_hub/manual/texi2html --init-file /cygdrive/c/Sandbox/damen_sensor_hub/manual/manual.init --css-ref=/cygdrive/c/Sandbox/damen_sensor_hub/manual/styles/styles.css --split=chapter --use-nodes --top-file=index.html --output=/cygdrive/c/Sandbox/damen_sensor_hub/manual/manual/html /cygdrive/c/Sandbox/damen_sensor_hub/manual/manual.texi	
	cp /cygdrive/c/Sandbox/damen_sensor_hub/manual/images/* /cygdrive/c/Sandbox/damen_sensor_hub/manual/manual/html/images/
	cp /cygdrive/c/Sandbox/damen_sensor_hub/manual/styles/* /cygdrive/c/Sandbox/damen_sensor_hub/manual/manual/html/styles/

info: 
	makeinfo --output=/cygdrive/c/Sandbox/damen_sensor_hub/manual/manual/$(APP_NAME).info /cygdrive/c/Sandbox/damen_sensor_hub/manual/manual.texi

pdf: 
	texi2dvi --build-dir=/tmp --output=/cygdrive/c/Sandbox/damen_sensor_hub/manual/manual/$(APP_NAME).pdf --pdf /cygdrive/c/Sandbox/damen_sensor_hub/manual/manual.texi 
	
chm:
	/cygdrive/c/Sandbox/damen_sensor_hub/manual/texi2html --output=/cygdrive/c/Sandbox/damen_sensor_hub/manual/manual/chm --init-file /cygdrive/c/Sandbox/damen_sensor_hub/manual/chm.init --css-include=/cygdrive/c/Sandbox/damen_sensor_hub/manual/styles/chm.css --split=section manual.texi
	mkdir -p /cygdrive/c/Sandbox/damen_sensor_hub/manual/manual/chm/images
	mkdir -p /cygdrive/c/Sandbox/damen_sensor_hub/manual/manual/chm/styles
	cp /cygdrive/c/Sandbox/damen_sensor_hub/manual/images/* /cygdrive/c/Sandbox/damen_sensor_hub/manual/manual/chm/images
	cp /cygdrive/c/Sandbox/damen_sensor_hub/manual/styles/* /cygdrive/c/Sandbox/damen_sensor_hub/manual/manual/chm/styles
ifdef HHC_BINARY	
	-hhc `cygpath -d /cygdrive/c/Sandbox/damen_sensor_hub/manual/manual/chm/manual.hhp`
	cp /cygdrive/c/Sandbox/damen_sensor_hub/manual/manual/chm/manual.chm /cygdrive/c/Sandbox/damen_sensor_hub/manual/manual/$(APP_NAME).chm
else
	@echo "hhc binary (windows help compiler) not found"
endif
	

.PHONY: pdf info html chm
