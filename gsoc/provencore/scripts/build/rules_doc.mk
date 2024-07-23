#
# Copyright (c) 2017-2023, ProvenRun and/or its affiliates. All rights reserved.
#

ifndef BASE_DIR
    $(error BASE_DIR has to be set by calling Makefile !)
endif

# Expecting this Makefile is called (or included) after the definition of
# DOC_BUILD_DIR that must indicate where to generate files for the given doc
ifndef DOC_BUILD_DIR
    $(error DOC_BUILD_DIR has to be set by calling Makefile !)
endif

ifndef DOC_ROOT
DOC_ROOT := $(BASE_DIR)/doc
endif

include $(BASE_DIR)/scripts/build/verbose.mk

DIAGRAMS_SRC_DIR   := diagrams
DIAGRAMS_BUILD_DIR := $(DOC_BUILD_DIR)/diagrams
DIAGRAMS_PREBUILT  := $(shell find "$(DIAGRAMS_SRC_DIR)" -type f ! \( -name "*.tex" -o -name ".*" \) 2> /dev/null)
ifeq ($(LATEX_QUIET),)
PDFLATEX           := pdflatex -interaction errorstopmode -file-line-error -halt-on-error
else
PDFLATEX           := pdflatex -interaction batchmode -file-line-error
endif

do_doc:

$(DOC_BUILD_DIR):
	$(Q)mkdir -p $@

$(DIAGRAMS_BUILD_DIR):
	$(Q)mkdir -p $@

# $1: absolute path of diagram source
# $2: diagram basename
define INSTALL_DIAGRAM

$(DIAGRAMS_BUILD_DIR)/$(2): $(1) | $(DIAGRAMS_BUILD_DIR)
	@echo "  DIAGRAM $$<" $(ECHO_OUTPUT)
	$(Q)mkdir -p $$(dir $$@)
	$(Q)ln -s $$< $$@

do_doc: $(DIAGRAMS_BUILD_DIR)/$(2)

DIAGRAM_PDFS += $(DIAGRAMS_BUILD_DIR)/$(2)

endef

# $1: absolute path of diagram source
# $2: diagram basename
define BUILD_DIAGRAM

$(DIAGRAMS_BUILD_DIR)/$(2).pdf: $(1) | $(DIAGRAMS_BUILD_DIR)
	@echo "  DIAGRAM $$<" $(ECHO_OUTPUT)
	$(Q)$(PDFLATEX) -output-directory $(DIAGRAMS_BUILD_DIR) $$< $(LATEX_OUTPUT)

do_doc: $(DIAGRAMS_BUILD_DIR)/$(2).pdf

DIAGRAM_PDFS += $(DIAGRAMS_BUILD_DIR)/$(2).pdf

endef

# $1: MD source file local path
# $2: MD file prefix
define BUILD_SLIDES_FROM_MD

# Note SOURCE_DATE_EPOCH is set for reproducible builds
# It is not enough, so the template adds `pdftrailerid`.
$(DOC_BUILD_DIR)/$(2).pdf: $(1) $(DIAGRAM_PDFS) | $(DOC_BUILD_DIR)
	@echo "    PANDOC $$<" $(ECHO_OUTPUT)
	$(Q)( cd $(DOC_BUILD_DIR) && \
		SOURCE_DATE_EPOCH=`$(BASE_DIR)/scripts/build/get_last_mod_time.sh --round-date "$$<"` \
		pandoc --data-dir=$(DOC_ROOT) --to=beamer --template=beamer.provencore \
		--variable=datadir=$(DOC_ROOT) --listings -o $$@ $$< )
	@echo "  DOC $$@" $(ECHO_OUTPUT)

do_doc: $(DOC_BUILD_DIR)/$(2).pdf

endef

# $1: MD source file local path
# $2: MD file prefix
define BUILD_PDF_FROM_MD

# Note SOURCE_DATE_EPOCH is set for reproducible builds
# It is not enough, so the template adds `pdftrailerid`.
$(DOC_BUILD_DIR)/$(2).pdf: $(1) $(DIAGRAM_PDFS) | $(DOC_BUILD_DIR)
	@echo "    PANDOC $$<" $(ECHO_OUTPUT)
	$(Q)( cd $(DOC_BUILD_DIR) && \
		SOURCE_DATE_EPOCH=`$(BASE_DIR)/scripts/build/get_last_mod_time.sh --round-date "$$<"` \
		pandoc --data-dir=$(DOC_ROOT) --template=default.provencore \
		--from markdown+footnotes \
		--variable=datadir=$(DOC_ROOT) --listings -o $$@ $$< )
	@echo "  DOC $$@" $(ECHO_OUTPUT)

do_doc: $(DOC_BUILD_DIR)/$(2).pdf

endef

# $1: TEX source file local path
# $2: TEX file prefix
# $3: doc folder
define BUILD_PDF_FROM_TEX

# Hack: our final wanted file is the $(2).pdf one.
# However, we generate intermediate $(2).pdf files, which is not Make-friendly
# Thus, the track built completion with $(2).pdf.ok:
#  !! you must delete $(2).pdf.ok before attempting to create a $(2).pdf file.
$(DOC_BUILD_DIR)/$(2).pdf.ok: $(DOC_BUILD_DIR)/$(2).pdf
	$(Q)touch $$@

# Note SOURCE_DATE_EPOCH is set for reproducible builds
$(DOC_BUILD_DIR)/$(2).pdf: $(1) $($(3)_PACKAGES) $(DOC_BUILD_DIR)/$(2).bib.ok
	$(Q)rm -f $(DOC_BUILD_DIR)/$(2).pdf.ok
	@echo "    PDFLATEX (STEP1)  $$<" $(ECHO_OUTPUT)
	$(Q)$(PDFLATEX) -output-directory $(DOC_BUILD_DIR) $$< $(LATEX_OUTPUT)
	@echo "    PDFLATEX (STEP2)  $$<" $(ECHO_OUTPUT)
	$(Q)SOURCE_DATE_EPOCH=`$(BASE_DIR)/scripts/build/get_last_mod_time.sh --round-date "$$<"` \
	    $(PDFLATEX) -output-directory $(DOC_BUILD_DIR) $$< $(LATEX_OUTPUT)

ifneq ($($(2).tex_bib), )
$(DOC_BUILD_DIR)/$(2).bib.ok: $($(2).tex_bib) $(1) $(DIAGRAM_PDFS) $(DIAGRAM_DEPS) | $(DOC_BUILD_DIR)
	$(Q)rm -f $$@ $(DOC_BUILD_DIR)/$(2).pdf.ok
	@echo "    PDFLATEX      $(1)" $(ECHO_OUTPUT)
	$(Q)$(PDFLATEX) -output-directory $(DOC_BUILD_DIR) $(1) $(LATEX_OUTPUT)
	@echo "    BIBTEX      $$<" $(ECHO_OUTPUT)
	$(Q)cp $$< $(DOC_BUILD_DIR)
	$(Q)( cd $(DOC_BUILD_DIR) && bibtex $(2) $(LATEX_OUTPUT) )
	@echo "    MAKEINDEX      $(2)" $(ECHO_OUTPUT)
	$(Q)( cd $(DOC_BUILD_DIR) && \
		find . -type f -name "*.idx" -exec makeindex -q {} \; )
	$(Q)( cd $(DOC_BUILD_DIR) && \
	      makeindex $(LATEX_QUIET) -t $(2).glg -s $(2).ist -o $(2).gls $(2).glo )
	$(Q)( cd $(DOC_BUILD_DIR) && \
	      makeindex $(LATEX_QUIET) -t $(2).alg -s $(2).ist -o $(2).acr $(2).acn )
	$(Q)touch $$@
else
$(DOC_BUILD_DIR)/$(2).bib.ok: | $(DOC_BUILD_DIR)
	$(Q)touch $$@
endif

do_doc: $(DOC_BUILD_DIR)/$(2).pdf.ok

endef

# Call rule to build targets to install prebuilt diagrams:
$(eval $(foreach obj,$(DIAGRAMS_PREBUILT),\
	$(call INSTALL_DIAGRAM,$(abspath $(obj)),$(notdir $(obj)))))

# Call rule to build targets to generate .pdf from .tex local diagram source:
$(eval $(foreach obj,$($(MAIN_DOC)_DIAGRAMS),\
	$(call BUILD_DIAGRAM,$(abspath $(obj)),$(basename $(notdir $(obj))))))

# Call rule to build targets to generate .pdf from .md source (slides):
$(eval $(foreach obj,$($(MAIN_DOC)_MD_SLIDES),\
	$(call BUILD_SLIDES_FROM_MD,$(abspath $(obj)),$(basename $(notdir $(obj))))))

# Call rule to build targets to generate .pdf from .md source:
$(eval $(foreach obj,$($(MAIN_DOC)_MD_FILES),\
	$(call BUILD_PDF_FROM_MD,$(abspath $(obj)),$(basename $(notdir $(obj))))))

# Call rule to build targets to generate .pdf from .tex source:
$(eval $(foreach obj,$($(MAIN_DOC)_TEX_FILES),\
	$(call BUILD_PDF_FROM_TEX,$(obj),$(basename $(notdir $(obj))),$(MAIN_DOC))))

# Adding rule to generate Doxygen documentation:
ifneq ($($(MAIN_DOC)_DOXYGEN_INPUT),)
do_doxygen: $(DOXYFILE) | $(DOC_BUILD_DIR)
	@echo "  DOXYGEN $(DOXYFILE)"
	$(Q)(cd $(DOC_BUILD_DIR) && doxygen $(DOXYFILE))

do_doc: do_doxygen
endif

do_clean_doc:
	@echo "  CLEAN $(MAIN_DOC) DOC" $(ECHO_OUTPUT)
	$(Q)rm -rf $(DOC_BUILD_DIR)

clean: do_clean_doc

.PHONY: do_clean_doc
