all: font16.inc font8.inc

font16.inc: font16.py
	python font16.py | xxd -i > font16.inc

font8.inc: font8.py
	python font8.py | xxd -i > font8.inc
