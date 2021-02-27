#include "nlwrapper.h"

NlAddr::NlAddr(RtNlLink &link) {
	data = nullptr;
	if (*link)
		data = rtnl_link_get_addr(*link);
	if (!data) error = -ENOENT;
}

int ifaceName2i(nl_cache_t cache, const char *name)
{
	if (cache && name)
		return rtnl_link_name2i(cache, name);
	return 0;
}

char *i2ifaceName(nl_cache_t cache, int index, char *buf, size_t sz)
{
	if (cache && buf)
		return rtnl_link_i2name(cache, index, buf, sz);
	return 0;
}

uint8_t maskToPrefix(const char *trgMask)
{
	uint8_t ret;
	uint32_t prefix = 0;
	uint32_t mask = 0;
	if (trgMask) {
		if (strlen(trgMask) <= 2) {
			try {
				prefix = atoi(trgMask);
			} catch (std::exception &ex) {
				prefix = 32;
			}
			mask = (0xFFFFFFFF << (32 - prefix)) & 0xFFFFFFFF;
			mask = htonl(mask);
		} else {
			mask = ntohl(inet_addr(trgMask));
			for (int i = 31; i >= 0; --i) {
				if (mask & (1 << i)) {
					prefix++;
				} else {
					break;
				}
			}
		}
	}
	ret = (uint8_t)prefix;
	return ret;
}

uint32_t getBroadcast(const char *ip, const char *mask)
{
	uint32_t ret = 0;
	if (ip && mask) {
		ret = htonl(inet_addr(ip));
		if (ret) {
			uint8_t prefix = maskToPrefix(mask);
			for (int i = 0; i < 32-prefix; ++i) {
				ret |= (1 << i);
			}
		}
		ret = htonl(ret);
	}
	return ret;
}

int Q_DECL_UNUSED prefixToMask(int prefix, char *buf)
{
	if (prefix >= 0 && prefix <= 32) {
		uint32_t mask = prefix ? (0xFFFFFFFF << (32 - prefix)) & 0xFFFFFFFF : 0;
		mask = htonl(mask);
		return inet_ntop(AF_INET, &mask, buf, INET_ADDRSTRLEN)? 0 : -1;
	}
	return 0;
}
