all: netw_srv.png netw_p2p.png netw_mixed.png

%.png: %.dot
	fdp $< -Tpng > $@

netw_srv.dot: netw.dot
	cat $< | sed -e 's/\/\/srv//g' > $@

netw_p2p.dot: netw.dot
	cat $< | sed -e 's/\/\/p2p//g' > $@

netw_mixed.dot: netw.dot
	cat $< | sed -e 's/\/\/mixed//g' > $@
