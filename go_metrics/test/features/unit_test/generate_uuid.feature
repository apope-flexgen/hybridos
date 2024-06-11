@unit_test @generate_uuid
Feature: generate_uuid

  # Ensure only sparse posts are sent
  Scenario: uuid for each output
    Given I am listening for a fims pub on /some/output
    Then I expect a fims pub to /some/output containing
      """
      {"output1bobcat":{"uuid":"429682ad-c00e-4f33-943b-c18bf76c994f","value":false},"output1cheetah":{"uuid":"49af5cab-2ded-4a14-84b4-042503c2e7ba","value":false},"output1lion":{"uuid":"5e9ebc97-38cf-4e3d-8cf2-635086c2162e","value":false},"output2bobcat":{"uuid":"5575697e-fced-4db7-ab5c-10270f258237","value":false},"output2cheetah":{"uuid":"0fb18bc0-ff60-4a30-a3ef-30ac068892c0","value":false},"output2lion":{"uuid":"7674efa5-9d04-4bca-b3f7-fc87d096d808","value":false},"output3bobcat":{"uuid":"4b8aceab-e091-4cae-a38d-0c3c541a0d38","value":false},"output3cheetah":{"uuid":"2b0ccb33-74c3-4570-a436-7242326b9469","value":false},"output3lion":{"uuid":"c976ef37-877c-48dc-aec9-fc2d7d878434","value":false}}
      """
