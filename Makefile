include mk/rules.mk
include mk/objective.mk

SUBDIRS = src lib

clean-posthook:
	-rm config.status config.log
