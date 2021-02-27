#ifndef SSDP_H
#define SSDP_H

#include <QString>

namespace SSDP {
	struct Params {
		union {
			struct {
				quint32 multi : 1;		//!< поддержка мультиподключения
				quint32 busy : 1;		//!< признак занятости модуля к подключению
				quint32 blockCon : 1;	//!< признак блокировки модуля на подключение
				quint32 blockReset : 1;	//!< признак блокировки модуля на сброс
			};
			quint32 b;
		} status;			//!< статус модуля
		QString name;		//!< имя модуля
		QString version;	//!< версия модуля
		QString ident;		//!< идентификация модуля
		QString type;		//!< тип модуля
							//!<	формат представления “P1,P2,P3,P4,P5” – где
							//!<	P1 – класс модуля
							//!<	P2 – тип модуля
							//!<	P3 – исполнение модуля
							//!<	P4 – ревизия модуля
							//!<	P5 – категория модуля
		QString extdata;	//!< расширенные данные
	};

	#define SERVER_PORT		1901
	#define SERVER_GROUP	"239.255.255.251"
}

#endif // SSDP_H
