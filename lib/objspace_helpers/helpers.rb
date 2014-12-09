class ObjspaceHelpers
  def self.dump_all_addresses
    _dump_addresses
  end

  def self.info_for_address(addresses)
    _addresses_to_info(addresses)
  end

  def self.info_for_obj(obj)
    _addresses_to_info([obj.object_id]).values.first
  end

  def self.diff(&block)
    objs_before = dump_all_addresses
    block.call
    objs_after = dump_all_addresses
    objs_after - objs_before - [objs_after.object_id]
  end
end
