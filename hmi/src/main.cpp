#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDir>

#include <initializer_list>
#include <signal.h>
#include <unistd.h>

#include "hmiworker.h"
#include "eventeater.h"
#include "hmiguiprovider.h"


static Q_DECL_UNUSED void ignoreUnixSignals(std::initializer_list<int> ignoreSignals)
{
	for (int sig : ignoreSignals)
		signal(sig, SIG_IGN);
}


static Q_DECL_UNUSED void catchUnixSignals(std::initializer_list<int> quitSignals)
{
	auto handler = [](int sig){
		Q_UNUSED(sig)
		QCoreApplication::exit(0);
	};

	sigset_t blocking_mask;
	sigemptyset(&blocking_mask);
	for (auto sig : quitSignals)
		sigaddset(&blocking_mask, sig);

	struct sigaction sa;
	bzero(&sa, sizeof(sa));
	sa.sa_handler = handler;
	sa.sa_mask = blocking_mask;

	for (auto sig : quitSignals)
		sigaction(sig, &sa, nullptr);
}


int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

	catchUnixSignals({SIGINT, SIGABRT, SIGHUP, SIGTERM, SIGQUIT, SIGKILL});

	{   QDir dir(QStringLiteral(CONFIG_WORKING_DIR));
        dir.mkpath(".");
	}
	{   QDir dir(QStringLiteral(CONFIG_PARAMS_FILE_DIR));
        dir.mkpath(".");
	}
	{   QDir dir(QStringLiteral(CONFIG_SHARE_DIR));
        dir.mkpath(".");
	}
	{   QDir dir(QStringLiteral(CONFIG_RUN_DIR));
        dir.mkpath(".");
	}
	{   QDir dir(QStringLiteral(CONFIG_LOG_DIR));
        dir.mkpath(".");
	}

	QGuiApplication app(argc, argv);

	EventEater *ee = new EventEater(&app);
	app.installEventFilter(ee);

	//HMIGuiProvider *gui = new HMIGuiProvider(&app);

	//QThread *thread = new QThread();
	//thread->setStackSize(CONFIG_THREAD_STACK_SIZE);
	HMIWorker *worker = new HMIWorker(&app);
	QObject::connect(ee, &EventEater::inputEventOccurred, worker, &HMIWorker::onInputEvent);
	//worker->moveToThread(thread);
	//QObject::connect(thread, &QThread::started, worker, &HMIWorker::start);
	//thread->start();
	worker->start();

	return app.exec();
}
