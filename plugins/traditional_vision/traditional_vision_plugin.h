#pragma once

#include "algorithm_platform/plugin_contract.h"

class TraditionalVisionPlugin final : public atp::IAlgorithmPlugin
{
public:
    atp::AlgorithmMeta meta() const override;
    bool initialize(QString* errorMessage) override;
    atp::AlgorithmResult run(const atp::AlgorithmRequest& request) override;

private:
    bool _initialized = false;
};
