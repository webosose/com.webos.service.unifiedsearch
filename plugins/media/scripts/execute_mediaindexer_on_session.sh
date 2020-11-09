#!/bin/sh

TARGET_USER=$1

echo "1. touch conf files and reload LS2"
sed -i 's/media"/media\/cryptofs\/apps"/g' /etc/palm/sandbox.conf
sed -i 's/mediaindexer/_mediaindexer/g' /usr/share/luna-service2/session-services.conf
ls-control scan-services

echo "2. restart media db on ${TARGET_USER}"
su ${TARGET_USER} -c "/usr/bin/restart db8-mediadb"

echo "3. launch mediaindexer manually on ${TARGET_USER} and wait for DB update (10sec)"
su ${TARGET_USER} -l -c "com.webos.service.mediaindexer & sleep 10 && kill %1;"

echo "4. Add title/artist indexes to the audio kind"
luna-send -c ${TARGET_USER} -n 1 -a com.palm.configurator luna://com.webos.mediadb/putKind '{
        "id":"com.webos.service.mediaindexer.audio:1",
        "owner":"com.webos.service.mediaindexer.media",
        "indexes":[
                {"name": "index1", "props": [{"name": "uri"}]},
                {"name": "index2", "props": [{"name": "dirty"}]},
                {"name": "index3", "props": [{"name": "uri"},{"name":"dirty"}]},
                {"name":"title", "props":[{"name":"title", "tokenize":"all", "collate":"primary"}]},
                {"name":"artist", "props":[{"name":"artist", "tokenize":"all", "collate":"primary"}]}
        ]
}'

echo "5. Give DB8 access permission to unifiedsearch"
luna-send -c ${TARGET_USER} -n 1 -a com.palm.configurator luna://com.webos.mediadb/putPermissions '{
    "permissions": [
        {
            "type": "db.kind",
            "object": "com.webos.service.mediaindexer.audio:1",
            "caller": "com.webos.service.unifiedsearch",
            "operations": {
                "read": "allow"
            },
        }
    ]
}'

echo "6. restart unifiedsearch on ${TARGET_USER}"
su ${TARGET_USER} -c "/usr/bin/restart unifiedsearch"

echo "Done: ready to check media plugin!"
