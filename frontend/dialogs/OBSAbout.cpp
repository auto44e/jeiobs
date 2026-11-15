#include "OBSAbout.hpp"

#include <widgets/OBSBasic.hpp>

#include <qt-wrappers.hpp>

#include "moc_OBSAbout.cpp"

/* JeiOBS fork links (see docs/jeiobs/NOTICE.md). */
#define JEIOBS_REPO_URL "https://github.com/auto44e/jeiobs"
#define JEIOBS_NOTICE_URL JEIOBS_REPO_URL "/blob/main/docs/jeiobs/NOTICE.md"
#define OBS_UPSTREAM_URL "https://github.com/obsproject/obs-studio"

extern bool steam;

OBSAbout::OBSAbout(QWidget *parent) : QDialog(parent), ui(new Ui::OBSAbout)
{
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	ui->setupUi(this);

	QString bitness;

	if (sizeof(void *) == 4) {
		bitness = " (32 bit)";
	} else if (sizeof(void *) == 8) {
		bitness = " (64 bit)";
	}

	QString ver = obs_get_version_string();

	ui->version->setText(ver + bitness);

	ui->contribute->setText(QTStr("About.Contribute"));

	/* JeiOBS: link to the fork, not to the OBS Project's donation and
	 * contribution pages. The upstream OBS Studio project is still credited
	 * (see About.Fork.Upstream and the Authors tab). */
	if (steam) {
		delete ui->donate;
	} else {
		ui->donate->setText("&nbsp;&nbsp;<a href='" JEIOBS_REPO_URL "'>" + QTStr("About.Fork.Repository") +
				    "</a>&nbsp;&nbsp;<a href='" JEIOBS_NOTICE_URL "'>" + QTStr("About.Fork.Notice") +
				    "</a>");
		ui->donate->setTextInteractionFlags(Qt::TextBrowserInteraction);
		ui->donate->setOpenExternalLinks(true);
	}

	ui->getInvolved->setText("&nbsp;&nbsp;<a href='" OBS_UPSTREAM_URL "'>" + QTStr("About.Fork.Upstream") + "</a>");
	ui->getInvolved->setTextInteractionFlags(Qt::TextBrowserInteraction);
	ui->getInvolved->setOpenExternalLinks(true);

	ui->about->setText("<a href='#'>" + QTStr("About") + "</a>");
	ui->authors->setText("<a href='#'>" + QTStr("About.Authors") + "</a>");
	ui->license->setText("<a href='#'>" + QTStr("About.License") + "</a>");

	ui->name->setProperty("class", "text-heading");
	ui->version->setProperty("class", "text-large");
	ui->about->setProperty("class", "bg-base");
	ui->authors->setProperty("class", "bg-base");
	ui->license->setProperty("class", "bg-base");
	ui->info->setProperty("class", "");

	connect(ui->about, &ClickableLabel::clicked, this, &OBSAbout::ShowAbout);
	connect(ui->authors, &ClickableLabel::clicked, this, &OBSAbout::ShowAuthors);
	connect(ui->license, &ClickableLabel::clicked, this, &OBSAbout::ShowLicense);

	/* JeiOBS: upstream fetched the OBS Project's Patreon supporter list from
	 * obsproject.com here. A fork must not present upstream's donors as its
	 * own supporters, nor contact upstream servers when the dialog opens, so
	 * the default "About" view shows the fork notice instead. */
	ShowAbout();
}

void OBSAbout::ShowAbout()
{
	QString text;

	text += "<h1>" + QTStr("About.Fork.Heading").toHtmlEscaped() + "</h1>";
	text += "<p style=\"font-size:14px;\">";
	text += QTStr("About.Fork.Body").toHtmlEscaped().replace("\n", "<br/>");
	text += "</p>";

	ui->textBrowser->setHtml(text);
}

void OBSAbout::ShowAuthors()
{
	std::string path;
	QString error = QTStr("About.Error").arg(JEIOBS_REPO_URL "/blob/main/AUTHORS");

#ifdef __APPLE__
	if (!GetDataFilePath("AUTHORS", path)) {
#else
	if (!GetDataFilePath("authors/AUTHORS", path)) {
#endif
		ui->textBrowser->setPlainText(error);
		return;
	}

	ui->textBrowser->setPlainText(QString::fromStdString(path));

	BPtr<char> text = os_quick_read_utf8_file(path.c_str());

	if (!text || !*text) {
		ui->textBrowser->setPlainText(error);
		return;
	}

	ui->textBrowser->setPlainText(QT_UTF8(text));
}

void OBSAbout::ShowLicense()
{
	std::string path;
	QString error = QTStr("About.Error").arg(JEIOBS_REPO_URL "/blob/main/COPYING");

	if (!GetDataFilePath("license/gplv2.txt", path)) {
		ui->textBrowser->setPlainText(error);
		return;
	}

	BPtr<char> text = os_quick_read_utf8_file(path.c_str());

	if (!text || !*text) {
		ui->textBrowser->setPlainText(error);
		return;
	}

	ui->textBrowser->setPlainText(QT_UTF8(text));
}
