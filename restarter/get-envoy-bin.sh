docker pull envoyproxy/envoy:latest
docker create --name envoyproxy envoyproxy/envoy
docker cp envoyproxy:/usr/local/bin/envoy ./
