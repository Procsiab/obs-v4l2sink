#include "v4l2sinkproperties.h"
#include "ui_v4l2sinkproperties.h"
#include <obs-frontend-api.h>
#include <util/config-file.h>
#include <linux/videodev2.h>

V4l2sinkProperties::V4l2sinkProperties(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::V4l2sinkProperties)
{
	ui->setupUi(this);
	connect(ui->pushButton_start,SIGNAL(clicked(bool)), this, SLOT(onStart()));
	connect(ui->pushButton_stop,SIGNAL(clicked(bool)), this, SLOT(onStop()));

	config_t* config = obs_frontend_get_global_config();
	config_set_default_bool(config, "V4l2sink", "AutoStart", false);
	config_set_default_string(config, "V4l2sink", "DevicePath", "/dev/video0");
	config_set_default_string(config, "V4l2sink", "Format", V4L2SINK_NV12);

	bool autostart = config_get_bool(config, "V4l2sink", "AutoStart");
	const char* device = config_get_string(config, "V4l2sink", "DevicePath");
	const char* format = config_get_string(config, "V4l2sink", "Format");

	ui->checkBox_auto->setChecked(autostart);
	ui->lineEdit_dev->setText(device);
	
	ui->comboBox_format->addItem(V4L2SINK_NV12, V4L2SINK_NV12);
	ui->comboBox_format->addItem(V4L2SINK_YUV420, V4L2SINK_YUV420);
	ui->comboBox_format->addItem(V4L2SINK_YUY2, V4L2SINK_YUY2);
#ifdef V4L2_PIX_FMT_ABGR32
	ui->comboBox_format->addItem(V4L2SINK_RGB32, V4L2SINK_RGB32);
#endif
	ui->comboBox_format->setCurrentIndex(
		ui->comboBox_format->findText(format));

	enableStart(true);

}

V4l2sinkProperties::~V4l2sinkProperties()
{
	v4l2sink_release();
	delete ui;
}

void V4l2sinkProperties::onStart()
{
	QByteArray ba_format = ui->comboBox_format->currentData().toString().toUtf8();
	QByteArray ba_dev_name = ui->lineEdit_dev->text().toUtf8();
	signal_handler_t *handler = v4l2sink_get_signal_handler();
	signal_handler_connect(handler, "v4l2close", output_stopped , this);
	enableStart(false);
	v4l2sink_enable(ba_dev_name.constData(), ba_format.constData());
}

void V4l2sinkProperties::onStop()
{
	v4l2sink_disable();
}

void V4l2sinkProperties::enableStart(bool enable)
{
	ui->pushButton_start->setEnabled(enable);
	ui->pushButton_stop->setEnabled(!enable);
}

static void output_stopped(void *data, calldata_t *cd)
{
	auto page = (V4l2sinkProperties*) data;
	auto output = (obs_output_t*) calldata_ptr(cd, "output");
	signal_handler_t *handler = obs_output_get_signal_handler(output);
	page->enableStart(true);
	signal_handler_disconnect(handler, "v4l2close", output_stopped , page);
}




