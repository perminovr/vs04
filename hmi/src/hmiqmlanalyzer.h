#ifndef HMIQMLANALYZER_H
#define HMIQMLANALYZER_H

#include <QFile>

class HMIQmlAnalyzer {
public:
	HMIQmlAnalyzer() = default;
	~HMIQmlAnalyzer() = default;

	bool fileIsOk(QFile &file, QString &error) {
		if (findFuncInAppWin(file)) {
			error = QStringLiteral("any functions in ApplicationWindow object are not allowed. "
					"You must place functions in child objects");
			return false;
		}
		error = "";
		return true;
	}

private:
	/*!
	 * @fn findFuncInAppWin
	 * @brief Необходимо проверять файл на наличие объявленных функций в ApplicationWindow.
	 *  Наличие таких функций часто ведет к падению программы с ошибкой
	 * "dbm_buffer_destroy: Assertion `!buf->ptr' failed".
	 * Версия psdk: tisdk-processor-sdk-06.01.00.08
	*/
	bool findFuncInAppWin(QFile &file) {
		if (file.isOpen()) {
			auto fdata = file.readAll();
			const char *d = fdata.data();
			const char *appWin = strstr(d, "ApplicationWindow");
			if (appWin) {
				const char *appWinOpenBrace = strstr(d, "{");
				if (appWinOpenBrace) {
					const char *p = appWinOpenBrace;
					int openCnt = 0;
					while (p && *p) {
						switch (*p) {
						case '{': openCnt++; break;
						case '}': openCnt--; break;
						case '/': { // comments
							switch (*(p+1)) {
							case '/': {
								while ((p && p+1 && *(p+1))   && (*p != '\n')) { p++; }
							} break;
							case '*': {
								while ((p && p+1 && *(p+1))   && !(*p == '*' && *(p+1) == '/')) { p++; }
							} break;
							default: break;
							}
						} break;
						case 'f': {
							if (memcmp(p, "function ", 9) == 0 && openCnt <= 1) {
								return true;
							}
						} break;
						default: break;
						}
						p++;
					}
				}
			}
		}
		return false;
	}
};

#endif // HMIQMLANALYZER_H
