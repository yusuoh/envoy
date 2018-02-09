#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "envoy/config/filter/http/ip_tagging/v2/ip_tagging.pb.h"
#include "envoy/http/filter.h"
#include "envoy/local_info/local_info.h"
#include "envoy/runtime/runtime.h"

#include "common/common/assert.h"

namespace Envoy {
namespace Http {

/**
 * Type of requests the filter should apply to.
 */
enum class FilterRequestType { Internal, External, Both };

/**
 * Configuration for the ip tagging filter.
 */
class IpTaggingFilterConfig {
public:
  //  IpTaggingFilterConfig(const Json::Object& json_config)
  //      : Json::Validator(json_config, Json::Schema::IP_TAGGING_HTTP_FILTER_SCHEMA),
  //        request_type_(stringToType(json_config.getString("request_type", "both"))) {}
  IpTaggingFilterConfig(const envoy::config::filter::http::ip_tagging::v2::IPTagging& config,
                        const LocalInfo::LocalInfo& local_info, Stats::Scope& scope,
                        Runtime::Loader& runtime)
      : fill_factor_(PROTOBUF_GET_WRAPPED_OR_DEFAULT(config, fill_factor, 0.5)),
        root_branching_factor_(PROTOBUF_GET_WRAPPED_OR_DEFAULT(config, root_branching_factor, 0)),
        request_type_(FilterRequestType::Both), local_info_(local_info), scope_(scope),
        runtime_(runtime) {
    // fixme make request_type_ take in config.request_type() and convert to string
    // fixme is this here where I validate the information? about the tags and valid cidr rangs
  }

  uint32_t rootBranchingFactor() const { return root_branching_factor_; }
  double fillFactor() const { return fill_factor_; }
  const LocalInfo::LocalInfo& localInfo() { return local_info_; }
  Runtime::Loader& runtime() { return runtime_; }
  Stats::Scope& scope() { return scope_; }
  FilterRequestType requestType() const { return request_type_; }

private:
  static FilterRequestType stringToType(const std::string& request_type) {
    if (request_type == "internal") {
      return FilterRequestType::Internal;
    } else if (request_type == "external") {
      return FilterRequestType::External;
    } else {
      ASSERT(request_type == "both");
      return FilterRequestType::Both;
    }
  }

  const double fill_factor_;
  const uint32_t root_branching_factor_;
  const FilterRequestType request_type_;
  const LocalInfo::LocalInfo& local_info_;
  Stats::Scope& scope_;
  Runtime::Loader& runtime_;
};

typedef std::shared_ptr<IpTaggingFilterConfig> IpTaggingFilterConfigSharedPtr;

/**
 * A filter that tags requests via the x-envoy-ip-tags header based on the request's trusted XFF
 * address.
 */
class IpTaggingFilter : public StreamDecoderFilter {
public:
  IpTaggingFilter(IpTaggingFilterConfigSharedPtr config);
  ~IpTaggingFilter();

  // Http::StreamFilterBase
  void onDestroy() override;

  // Http::StreamDecoderFilter
  FilterHeadersStatus decodeHeaders(HeaderMap& headers, bool end_stream) override;
  FilterDataStatus decodeData(Buffer::Instance& data, bool end_stream) override;
  FilterTrailersStatus decodeTrailers(HeaderMap& trailers) override;
  void setDecoderFilterCallbacks(StreamDecoderFilterCallbacks& callbacks) override;

private:
  IpTaggingFilterConfigSharedPtr config_;
  StreamDecoderFilterCallbacks* callbacks_{};
};

} // namespace Http
} // namespace Envoy
