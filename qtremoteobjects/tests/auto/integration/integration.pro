TEMPLATE = subdirs
SUBDIRS = local tcp

qnx: SUBDIRS += qnx

local.path = local
tcp.path = tcp
qnx.path = qnx

# QTBUG-61552
boot2qt: SUBDIRS -= local
