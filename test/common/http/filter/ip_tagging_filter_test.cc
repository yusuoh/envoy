#include "common/buffer/buffer_impl.h"
#include "common/http/filter/ip_tagging_filter.h"
#include "common/http/header_map_impl.h"
#include "common/http/headers.h"

#include "test/mocks/http/mocks.h"
#include "test/mocks/local_info/mocks.h"
#include "test/mocks/runtime/mocks.h"
#include "test/test_common/utility.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::Return;
using testing::_;

namespace Envoy {
namespace Http {

class IpTaggingFilterTest : public testing::Test {
public:
  const std::string internal_request_json = R"EOF(
    {
      "request_type" : "internal",
      "ip_tags" : [
        {
          "ip_tag_name" : "test_internal",
          "ip_list" : ["1.2.3.4"]
        }
      ]
    }
  )EOF";
  const std::string internal_request_yaml = R"EOF(
request_type: internal
ip_tags:
  - ip_tag_name: internal_request
    ip_list:
      - {address_prefix: 1.2.3.5, prefix_len: 32}

)EOF";
  const std::string external_request_json = R"EOF(
    {
      "request_type" : "external",
      "ip_tags" : [
        {
          "ip_tag_name" : "test_external",
          "ip_list" : ["1.2.3.4"]
        }
      ]
    }
  )EOF";
  const std::string external_request_yaml = R"EOF(
request_type: external
ip_tags:
  - ip_tag_name: external_request
    ip_list:
      - {address_prefix: 1.2.3.4, prefix_len: 32}
)EOF";
  const std::string both_request_json = R"EOF(
    {
      "request_type" : "both",
      "ip_tags" : [
        {
          "ip_tag_name" : "test_both",
          "ip_list" : ["1.2.3.4"]
        }
      ]
    }
  )EOF";

  const std::string both_request_yaml = R"EOF(
request_type: both
ip_tags:
  - ip_tag_name: external_request
    ip_list:
      - {address_prefix: 1.2.3.4, prefix_len: 32}
  - ip_tag_name: internal_request
    ip_list:
      - {address_prefix: 1.2.3.5, prefix_len: 32}
)EOF";

  const std::string lc_trie_values_yaml = R"EOF(
request_type: both
fill_factor: 0.25
root_branching_factor: 16
ip_tags:
  - ip_tag_name: external_request
    ip_list:
      - {address_prefix: 1.2.3.4, prefix_len: 33}
  - ip_tag_name: internal_request
    ip_list:
      - {address_prefix: 1.2.3.5, prefix_len: 32}

)EOF";
  void SetUpTest(const std::string& yaml) {
    envoy::config::filter::http::ip_tagging::v2::IPTagging config;
    envoy::api::v2::Listener listener;
    MessageUtil::loadFromYaml(yaml, config);
    config_.reset(new IpTaggingFilterConfig(config, local_info_, stats_store_, runtime_));
    filter_.reset(new IpTaggingFilter(config_));
    filter_->setDecoderFilterCallbacks(filter_callbacks_);

    // fixme reset request_headers_?
  }

  ~IpTaggingFilterTest() { filter_->onDestroy(); }

  IpTaggingFilterConfigSharedPtr config_;
  std::unique_ptr<IpTaggingFilter> filter_;
  NiceMock<MockStreamDecoderFilterCallbacks> filter_callbacks_;
  TestHeaderMapImpl request_headers_;
  Buffer::OwnedImpl data_;
  Stats::IsolatedStoreImpl stats_store_;
  NiceMock<Runtime::MockLoader> runtime_;
  NiceMock<LocalInfo::MockLocalInfo> local_info_;
};
// fixme if json is supported, then add a v1 to v2 conversion test
// fixme add ipv6 tests
// fixme add tests to append to the header
// fixme add request type checks
TEST_F(IpTaggingFilterTest, InternalRequest) {
  SetUpTest(internal_request_yaml);
  EXPECT_EQ(FilterRequestType::Internal, config_->requestType());
  EXPECT_EQ(FilterHeadersStatus::Continue, filter_->decodeHeaders(request_headers_, false));
  EXPECT_EQ("internal_request", request_headers_.get_(Headers::get().EnvoyIpTags));

  EXPECT_EQ(FilterDataStatus::Continue, filter_->decodeData(data_, false));
  EXPECT_EQ(FilterTrailersStatus::Continue, filter_->decodeTrailers(request_headers_));
}

TEST_F(IpTaggingFilterTest, ExternalRequest) {
  SetUpTest(external_request_yaml);
  EXPECT_EQ(FilterRequestType::External, config_->requestType());

  EXPECT_EQ(FilterHeadersStatus::Continue, filter_->decodeHeaders(request_headers_, false));
  EXPECT_EQ("external_request", request_headers_.get_(Headers::get().EnvoyIpTags));

  EXPECT_EQ(FilterDataStatus::Continue, filter_->decodeData(data_, false));
  EXPECT_EQ(FilterTrailersStatus::Continue, filter_->decodeTrailers(request_headers_));
}

TEST_F(IpTaggingFilterTest, BothRequest) {
  SetUpTest(both_request_yaml);
  EXPECT_EQ(FilterRequestType::Both, config_->requestType());

  EXPECT_EQ(FilterHeadersStatus::Continue, filter_->decodeHeaders(request_headers_, false));
  EXPECT_EQ("internal_request", request_headers_.get_(Headers::get().EnvoyIpTags));
  EXPECT_EQ("external_request", request_headers_.get_(Headers::get().EnvoyIpTags));

  EXPECT_EQ(FilterDataStatus::Continue, filter_->decodeData(data_, false));
  EXPECT_EQ(FilterTrailersStatus::Continue, filter_->decodeTrailers(request_headers_));
}

TEST_F(IpTaggingFilterTest, LcTrieValues) {
  SetUpTest(lc_trie_values_yaml);
  EXPECT_EQ(16u, config_->rootBranchingFactor());
  EXPECT_EQ(0.25, config_->fillFactor());
  EXPECT_EQ(FilterHeadersStatus::Continue, filter_->decodeHeaders(request_headers_, false));
  EXPECT_EQ(FilterDataStatus::Continue, filter_->decodeData(data_, false));
  EXPECT_EQ(FilterTrailersStatus::Continue, filter_->decodeTrailers(request_headers_));
}

} // namespace Http
} // namespace Envoy
