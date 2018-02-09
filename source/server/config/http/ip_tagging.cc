#include "server/config/http/ip_tagging.h"

#include <string>

#include "envoy/registry/registry.h"

#include "common/http/filter/ip_tagging_filter.h"
#include "common/json/config_schemas.h"

namespace Envoy {
namespace Server {
namespace Configuration {

HttpFilterFactoryCb createFilterFactoryFromProto(const Protobuf::Message& proto_config,
                                                 const std::string& stats_prefix,
                                                 FactoryContext& context) override;

HttpFilterFactoryCb IpTaggingFilterConfig::createFilterFactory(const Json::Object&,
                                                               const std::string&,
                                                               FactoryContext& context) {

  // FIXME return an error - not supproted in v1
  // if v1 is supported move this logic to a helper method
  envoy::config::filter::http::ip_tagging::v2::IPTagging proto_config;
  Http::IpTaggingFilterConfigSharedPtr config(new Http::IpTaggingFilterConfig(
      proto_config, context.localInfo(), context.scope(), context.runtime()));
  return [config](Http::FilterChainFactoryCallbacks& callbacks) -> void {
    callbacks.addStreamDecoderFilter(
        Http::StreamDecoderFilterSharedPtr{new Http::IpTaggingFilter(config)});
  };
}

HttpFilterFactoryCb
IpTaggingFilterConfig::createFilterFactoryFromProto(const Protobuf::Message& proto_config,
                                                    const std::string&, FactoryContext& context) {
  // Config::FilterJson::translateHttpRateLimitFilter(json_config, proto_config);
  // fixme what is local info for again? remote address?
  // fixme where do i check if ip_tags is specified the content is appropriately
  Http::IpTaggingFilterConfigSharedPtr config(new Http::IpTaggingFilterConfig(
      MessageUtil::downcastAndValidate<
          const envoy::config::filter::http::ip_tagging::v2::IPTagging&>(proto_config),
      context.localInfo(), context.scope(), context.runtime()));
  ));
  return [config](Http::FilterChainFactoryCallbacks& callbacks) -> void {
    callbacks.addStreamDecoderFilter(
        Http::StreamDecoderFilterSharedPtr{new Http::IpTaggingFilter(config)});
  };
}

/**
 * Static registration for the ip tagging filter. @see RegisterFactory.
 */
static Registry::RegisterFactory<IpTaggingFilterConfig, NamedHttpFilterConfigFactory> register_;

} // namespace Configuration
} // namespace Server
} // namespace Envoy
