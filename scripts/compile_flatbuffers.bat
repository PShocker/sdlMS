flatc --cpp --gen-mutable --gen-object-api --filename-suffix "" --filename-ext "h" ^
-o ../src/common/flatbuffers ^
../flatbuffers/client.fbs ^
../flatbuffers/server.fbs ^
../flatbuffers/common.fbs ^
../flatbuffers/protocol.fbs