class ObjspaceHelpers::TrackedObject
  def self.wrap(addresses, referenced_by = [])
    Array.new(addresses || []).map do |address|
      new(address, wrap(referenced_by[address]))
    end
  end

  def self.for_object(obj)
    new ObjspaceHelpers._address_of_obj(obj)
  end

  attr_reader :address
  attr_reader :referenced_by

  def initialize(address, referenced_by = [])
    @address = address
    @referenced_by = referenced_by
  end

  def info
    ObjspaceHelpers._addresses_to_info([@address]).values.first
  end

  def klass
    self.class.new(info['class'])
  end

  def dereference
    ObjspaceHelpers._obj_for_address(@address)
  end
end
