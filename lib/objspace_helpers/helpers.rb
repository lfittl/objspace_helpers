class ObjspaceHelpers
  def self.dump_all_addresses
    _dump_addresses
  end

  def self.info_for_address(addresses)
    _addresses_to_info(addresses)
  end

  def self.info_for_obj(obj)
    _addresses_to_info([address_of(obj)]).values.first
  end

  def self.obj_for(address)
    _obj_for_address(address)
  end

  def self.address_of(obj)
    _address_of_obj(obj)
  end

  def self.diff(&block)
    objs_before = dump_all_addresses
    block.call
    objs_after = dump_all_addresses
    objs_after - objs_before - [address_of(objs_after)]
  end
end
