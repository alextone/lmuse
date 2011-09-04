//=========================================================
//  MusE
//  Linux Music Editor
//    $Id: pastedialog.cpp,v 1.1.1.1 2011/05/05 18:51:04 flo93 Exp $
//  (C) Copyright 2011 Florian Jung (flo93@sourceforge.net)
//=========================================================

#include <QButtonGroup>
#include "pastedialog.h"
#include "xml.h"
#include "gconfig.h"

PasteDialog::PasteDialog(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);
	button_group = new QButtonGroup;
	button_group->addButton(merge_button,0);
	button_group->addButton(move_all_button,1);
	button_group->addButton(move_some_button,2);
	
	raster_spinbox->setSingleStep(config.division);
	
	connect(raster_spinbox, SIGNAL(valueChanged(int)), this, SLOT(raster_changed(int)));
	connect(n_spinbox, SIGNAL(valueChanged(int)), this, SLOT(number_changed(int)));
	
	pull_values();
}

void PasteDialog::pull_values()
{
	insert_method = button_group->checkedId();
	number = n_spinbox->value();
	raster = raster_spinbox->value();
	all_in_one_track = all_in_one_track_checkbox->isChecked();
	clone = clone_checkbox->isChecked();
}

void PasteDialog::accept()
{
	pull_values();
	QDialog::accept();
}

int PasteDialog::exec()
{
	if ((insert_method < 0) || (insert_method > 2)) insert_method=0;
	
	button_group->button(insert_method)->setChecked(true);
	n_spinbox->setValue(number);
	raster_spinbox->setValue(raster);
	all_in_one_track_checkbox->setChecked(all_in_one_track);
	clone_checkbox->setChecked(clone);
	
	return QDialog::exec();
}

QString PasteDialog::ticks_to_quarter_string(int ticks)
{
	if (ticks % config.division == 0)
	{
		return tr("%n quarter(s)", "", ticks/config.division);
	}
	else
	{
		double quarters = (double) ticks/config.division;
		bool one = ( quarters > 0.995 && quarters < 1.005 );
		if (one)
			return QString::number(quarters, 'f', 2) + " " + tr("quarter");
		else
			return QString::number(quarters, 'f', 2) + " " + tr("quarters");
	}
}

void PasteDialog::raster_changed(int r)
{
	raster_quarters->setText(ticks_to_quarter_string(r));
	insert_quarters->setText(ticks_to_quarter_string(r*n_spinbox->value()));
}

void PasteDialog::number_changed(int n)
{
	insert_quarters->setText(ticks_to_quarter_string(n*raster_spinbox->value()));
}


void PasteDialog::read_configuration(Xml& xml)
{
	for (;;)
	{
		Xml::Token token = xml.parse();
		if (token == Xml::Error || token == Xml::End)
			break;
			
		const QString& tag = xml.s1();
		switch (token)
		{
			case Xml::TagStart:
				if (tag == "insert_method")
					insert_method=xml.parseInt();
				else if (tag == "number")
					number=xml.parseInt();
				else if (tag == "raster")
					raster=xml.parseInt();
				else if (tag == "clone")
					clone=xml.parseInt();
				else if (tag == "all_in_one_track")
					all_in_one_track=xml.parseInt();
				else
					xml.unknown("PasteDialog");
				break;
				
			case Xml::TagEnd:
				if (tag == "pastedialog")
					return;
				
			default:
				break;
		}
	}
}

void PasteDialog::write_configuration(int level, Xml& xml)
{
	xml.tag(level++, "pastedialog");
	xml.intTag(level, "insert_method", insert_method);
	xml.intTag(level, "number", number);
	xml.intTag(level, "raster", raster);
	xml.intTag(level, "clone", clone);
	xml.intTag(level, "all_in_one_track", all_in_one_track);
	xml.tag(level, "/pastedialog");
}
