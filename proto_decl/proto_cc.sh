protoc -I=. --cpp_out=../proto $SRC_DIR ./*.proto
find ./ -type f | grep -i proto$ | xargs -i cp {} ../web/proto