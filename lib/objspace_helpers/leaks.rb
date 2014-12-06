class ObjspaceHelpers
  class << self
    def find_leaks(trace: false, &block)
      ObjectSpace::trace_object_allocations_start if trace

      objs_before = _dump_addresses

      block.call

      # Run GC twice to try to get rid of as much false positives as possible
      GC.start
      GC.start

      objs_after       = _dump_addresses

      ObjectSpace::trace_object_allocations_stop if trace

      leaked_addresses = objs_after - objs_before - [_address_of_obj(objs_after)]

      # Note (LukasFittl): 2x speedup if we move this to native code
      referenced_by = {}
      _addresses_to_references(objs_after).each do |addr, references|
        (references & leaked_addresses).each do |ref|
          referenced_by[ref] ||= []
          referenced_by[ref] << addr
        end
      end

      TrackedObject.wrap(leaked_addresses, referenced_by)
    end

    def find_leak_sources(trace: false, &block)
      leaks = find_leaks(trace: trace, &block)
      leaks_by_source = {}
      top_level_leaks = []

      leaks.each do |leak|
        if leak.referenced_by.empty?
          top_level_leaks << leak
        else
          leak.referenced_by.each do |source|
            leaks_by_source[source] ||= []
            leaks_by_source[source] << leak
          end
        end
      end

      [top_level_leaks, leaks_by_source]
    end
  end
end
