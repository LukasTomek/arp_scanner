import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

# Match the exact C++ namespace
arp_scanner_ns = cg.esphome_ns.namespace("arp_scanner")
ArpScannerComponent = arp_scanner_ns.class_("ArpScannerComponent", cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(ArpScannerComponent),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    # No source adding functions needed here! 
    # ESPHome automatically discovers and builds arp_scanner.cpp.
