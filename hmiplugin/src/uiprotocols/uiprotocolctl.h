// #ifndef UIPROTOCOLCTL_H
// #define UIPROTOCOLCTL_H

// #ifndef UIProtocolClassName
// #error "UIProtocolClassName must be specified"
// #endif

// #define UIProtocolCtlCfgClassName UIProtocolClassName ##ProtocolCtlConfig

// #include "uiprotocol.h"

// class UIProtocolClassName;
// class UIProtocolCtlCfgClassName : ProtocolCtlConfig {
// 	Q_OBJECT // todo ????
// public:
// 	UIProtocolClassName *ui;
// 	UIProtocolCtlCfgClassName(QObject *parent = nullptr) : ProtocolCtlConfig(parent), ui((UIProtocolClassName *)parent) { }
// 	virtual ~UIProtocolCtlCfgClassName() = default;
// 	virtual QByteArray toBytes() const override;
// 	virtual int fromBytes(const quint8 *array) override;
// };

// #define UIProtocolCtlCfgClass_Declare \
// 	friend class UIProtocolCtlCfgClassName; \
// 	UIProtocolCtlCfgClassName *p_uipctlcfg; \

// #define UIProtocolCtlCfgClass_construct() { \
// 	this->p_uipctlcfg = new UIProtocolCtlCfgClassName(this); \
// }

// #define UIProtocolCtlCfgClass_getPCtlCfg() ({ \
// 	this->p_uipctlcfg; \
// })

// #define UIProtocolCtlCfgClass_getUIProtocol() ({ \
// 	this->ui; \
// })

// #endif // UIPROTOCOLCTL_H
