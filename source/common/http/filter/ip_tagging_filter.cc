#include "common/http/filter/ip_tagging_filter.h"

namespace Envoy {
namespace Http {

IpTaggingFilter::IpTaggingFilter(IpTaggingFilterConfigSharedPtr config) : config_(config) {}

IpTaggingFilter::~IpTaggingFilter() {}

void IpTaggingFilter::onDestroy() {}

FilterHeadersStatus IpTaggingFilter::decodeHeaders(HeaderMap& headers, bool) {
  // fixme add internalvs external check
  // add trie
  std::string val{"test"};
  headers.insertEnvoyIpTags().value(val);
  return FilterHeadersStatus::Continue;
}

FilterDataStatus IpTaggingFilter::decodeData(Buffer::Instance&, bool) {
  return FilterDataStatus::Continue;
}

FilterTrailersStatus IpTaggingFilter::decodeTrailers(HeaderMap&) {
  return FilterTrailersStatus::Continue;
}

void IpTaggingFilter::setDecoderFilterCallbacks(StreamDecoderFilterCallbacks& callbacks) {
  callbacks_ = &callbacks;
}

} // namespace Http
} // namespace Envoy
