
#include <config.h>

#include "ParagraphParameters.h"
#include "ParameterStruct.h"

// Initialize static member var.
ShareContainer<ParameterStruct> ParagraphParameters::container;

ParagraphParameters::ParagraphParameters()
{
	ParameterStruct tmp;
	tmp.appendix = false;
	tmp.align = LYX_ALIGN_BLOCK;

	set_from_struct(tmp);
}


void ParagraphParameters::clear()
{
	ParameterStruct tmp(*param);
	tmp.line_top = false;
	tmp.line_bottom = false;
	tmp.pagebreak_top = false;
	tmp.pagebreak_bottom = false;
	tmp.added_space_top = VSpace(VSpace::NONE);
	tmp.added_space_bottom = VSpace(VSpace::NONE);
	tmp.spacing.set(Spacing::Default);
	tmp.align = LYX_ALIGN_LAYOUT;
	tmp.depth = 0;
	tmp.noindent = false;
	tmp.labelstring.erase();
	tmp.labelwidthstring.erase();
	tmp.start_of_appendix = false;
#ifndef NO_PEXTRA_REALLY
        //tmp.pextra_type = PEXTRA_NONE;
        tmp.pextra_type = 0;
        tmp.pextra_width.erase();
        tmp.pextra_widthp.erase();
        //tmp.pextra_alignment = MINIPAGE_ALIGN_TOP;
        tmp.pextra_alignment = 0;
        tmp.pextra_hfill = false;
        tmp.pextra_start_minipage = false;
#endif
	set_from_struct(tmp);
}


bool ParagraphParameters::sameLayout(ParagraphParameters const & pp) const
{
	return param->align == pp.param->align &&
		param->line_bottom == pp.param->line_bottom &&
		param->pagebreak_bottom == pp.param->pagebreak_bottom &&
		param->added_space_bottom == pp.param->added_space_bottom &&

		param->line_top == pp.param->line_top &&
		param->pagebreak_top == pp.param->pagebreak_top &&
		param->added_space_top == pp.param->added_space_top &&
#ifndef NO_PEXTRA_REALLY
		param->spacing == pp.param->spacing &&
		param->pextra_type == pp.param->pextra_type &&
                param->pextra_width == pp.param->pextra_width && 
                param->pextra_widthp == pp.param->pextra_widthp &&
                param->pextra_alignment == pp.param->pextra_alignment && 
                param->pextra_hfill == pp.param->pextra_hfill && 
                param->pextra_start_minipage == pp.param->pextra_start_minipage &&
#endif
		param->noindent == pp.param->noindent &&
		param->depth == pp.param->depth;
}


void ParagraphParameters::makeSame(ParagraphParameters const & pp)
{
	ParameterStruct tmp(*param);
	tmp.align = pp.param->align;
	// tmp.labelwidthstring = pp.params.labelwidthstring;
	tmp.line_bottom = pp.param->line_bottom;
	tmp.pagebreak_bottom = pp.param->pagebreak_bottom;
	tmp.added_space_bottom = pp.param->added_space_bottom;
	tmp.line_top = pp.param->line_top;
	tmp.pagebreak_top = pp.param->pagebreak_top;
	tmp.added_space_top = pp.param->added_space_top;
	tmp.spacing = pp.param->spacing;
#ifndef NO_PEXTRA_REALLY
	tmp.pextra_type = pp.param->pextra_type;
	tmp.pextra_width = pp.param->pextra_width;
	tmp.pextra_widthp = pp.param->pextra_widthp;
	tmp.pextra_alignment = pp.param->pextra_alignment;
	tmp.pextra_hfill = pp.param->pextra_hfill;
	tmp.pextra_start_minipage = pp.param->pextra_start_minipage;
#endif
	tmp.noindent = pp.param->noindent;
	tmp.depth = pp.param->depth;

	set_from_struct(tmp);
}


void ParagraphParameters::set_from_struct(ParameterStruct const & ps) 
{
	// get new param from container with tmp as template
	param = container.get(ps);
}


VSpace const & ParagraphParameters::spaceTop() const
{
	return param->added_space_top;
}


void ParagraphParameters::spaceTop(VSpace const & vs)
{
	ParameterStruct tmp(*param);
	tmp.added_space_top = vs;
	set_from_struct(tmp);
}


VSpace const & ParagraphParameters::spaceBottom() const
{
	return param->added_space_bottom;
}


void ParagraphParameters::spaceBottom(VSpace const & vs)
{
	ParameterStruct tmp(*param);
	tmp.added_space_bottom = vs;
	set_from_struct(tmp);
}


Spacing const & ParagraphParameters::spacing() const
{
	return param->spacing;
}


void ParagraphParameters::spacing(Spacing const & s)
{
	ParameterStruct tmp(*param);
	tmp.spacing = s;
	set_from_struct(tmp);
}


bool ParagraphParameters::noindent() const
{
	return param->noindent;
}


void ParagraphParameters::noindent(bool ni)
{
	ParameterStruct tmp(*param);
	tmp.noindent = ni;
	set_from_struct(tmp);
}


bool ParagraphParameters::lineTop() const
{
	return param->line_top;
}


void ParagraphParameters::lineTop(bool lt)
{
	ParameterStruct tmp(*param);
	tmp.line_top = lt;
	set_from_struct(tmp);
}


bool ParagraphParameters::lineBottom() const
{
	return param->line_bottom;
}


void ParagraphParameters::lineBottom(bool lb)
{
	ParameterStruct tmp(*param);
	tmp.line_bottom = lb;
	set_from_struct(tmp);
}


bool ParagraphParameters::pagebreakTop() const
{
	return param->pagebreak_top;
}


void ParagraphParameters::pagebreakTop(bool pbt)
{
	ParameterStruct tmp(*param);
	tmp.pagebreak_top = pbt;
	set_from_struct(tmp);
}


bool ParagraphParameters::pagebreakBottom() const
{
	return param->pagebreak_bottom;
}


void ParagraphParameters::pagebreakBottom(bool pbb)
{
	ParameterStruct tmp(*param);
	tmp.pagebreak_bottom = pbb;
	set_from_struct(tmp);
}


LyXAlignment ParagraphParameters::align() const
{
	return param->align;
}


void ParagraphParameters::align(LyXAlignment la)
{
	ParameterStruct tmp(*param);
	tmp.align = la;
	set_from_struct(tmp);
}


char ParagraphParameters::depth() const
{
	return param->depth;
}


void ParagraphParameters::depth(char d)
{
	ParameterStruct tmp(*param);
	tmp.depth = d;
	set_from_struct(tmp);
}


bool ParagraphParameters::startOfAppendix() const
{
	return param->start_of_appendix;
}


void ParagraphParameters::startOfAppendix(bool soa)
{
	ParameterStruct tmp(*param);
	tmp.start_of_appendix = soa;
	set_from_struct(tmp);
}


bool ParagraphParameters::appendix() const
{
	return param->appendix;
}


void ParagraphParameters::appendix(bool a)
{
	ParameterStruct tmp(*param);
	tmp.appendix = a;
	set_from_struct(tmp);
}


string const & ParagraphParameters::labelString() const
{
	return param->labelstring;
}


void ParagraphParameters::labelString(string const & ls)
{
	ParameterStruct tmp(*param);
	tmp.labelstring = ls;
	set_from_struct(tmp);
}


string const & ParagraphParameters::labelWidthString() const
{
	return param->labelwidthstring;
}


void ParagraphParameters::labelWidthString(string const & lws)
{
	ParameterStruct tmp(*param);
	tmp.labelwidthstring = lws;
	set_from_struct(tmp);
}

#ifndef NO_PEXTRA_REALLY
int ParagraphParameters::pextraType() const
{
	return param->pextra_type;
}


void ParagraphParameters::pextraType(int t)
{
	ParameterStruct tmp(*param);
	tmp.pextra_type = t;
	set_from_struct(tmp);
}


string const & ParagraphParameters::pextraWidth() const
{
	return param->pextra_width;
}


void ParagraphParameters::pextraWidth(string const & w)
{
	ParameterStruct tmp(*param);
	tmp.pextra_width = w;
	set_from_struct(tmp);
}


string const & ParagraphParameters::pextraWidthp() const
{
	return param->pextra_widthp;
}


void ParagraphParameters::pextraWidthp(string const & wp)
{
	ParameterStruct tmp(*param);
	tmp.pextra_widthp = wp;
	set_from_struct(tmp);
}


int ParagraphParameters::pextraAlignment() const
{
	return param->pextra_alignment;
}


void ParagraphParameters::pextraAlignment(int a)
{
	ParameterStruct tmp(*param);
	tmp.pextra_alignment = a;
	set_from_struct(tmp);
}


bool ParagraphParameters::pextraHfill() const
{
	return param->pextra_hfill;
}


void ParagraphParameters::pextraHfill(bool hf)
{
	ParameterStruct tmp(*param);
	tmp.pextra_hfill = hf;
	set_from_struct(tmp);
}


bool ParagraphParameters::pextraStartMinipage() const
{
	return param->pextra_start_minipage;
}


void ParagraphParameters::pextraStartMinipage(bool smp)
{
	ParameterStruct tmp(*param);
	tmp.pextra_start_minipage = smp;
	set_from_struct(tmp);
}
#endif
