require 'spec_helper'

describe ObjspaceHelpers do
  subject { described_class }

  def addrs2objs(addrs)
    addrs.map {|r| subject.obj_for(r) }
  end

  describe '.dump_all_addresses' do
    it 'returns addresses' do
      test_obj = "mystring"

      x = subject.dump_all_addresses
      expect(x).not_to be_nil
      expect(x).to include subject.address_of(test_obj)
    end
  end

  describe '.diff' do
    it 'diffs objects created in a block' do
      obj = nil
      addresses = subject.diff do
        obj = "mystring"
      end

      obj_addr = subject.address_of(obj)

      expect(addresses).not_to be_nil
      expect(addresses).to eq [obj_addr]
      expect(addresses.size).to eq 1
    end
  end

  describe '.info_for_address' do
    def dump_block(&block)
      obj = nil
      addresses = subject.diff do
        obj = block.call
      end

      dump = subject.info_for_address(addresses)
      obj_addr = subject.address_of(obj)

      if info = dump[obj_addr]
        klass = subject.obj_for(info.delete('class'))
        refs  = addrs2objs info.delete('references')
        [info, klass, refs]
      else
        puts dump.inspect
        info
      end
    end

    it 'can dump string objects' do
      info, klass, refs = dump_block { "mystring" }

      expect(klass).to eq String
      expect(refs).to be_empty
      expect(info).to eq({
        "type"=>"STRING",
        "embedded"=>true,
        "bytesize"=>8,
        "value"=>"mystring",
        "encoding"=>"UTF-8",
        "flags"=>{"wb_protected"=>true}
      })
    end

    it 'can dump arrays' do
      info, klass, refs = dump_block { ['a', 42] }

      expect(klass).to eq Array
      expect(refs.size).to eq 1
      expect(refs.first).to eq 'a'
      expect(info).to eq({
        "type"=>"ARRAY",
        "length"=>2,
        "embedded"=>true,
        "flags"=>{"wb_protected"=>true}
      })
    end

    it 'can dump classes' do
      info, klass, refs = dump_block do
        class Something
        end
        Something
      end

      expect(refs).to eq ["Something", Object]
      expect(info).to eq({
        "type"=>"CLASS",
        "name"=>"Something",
        "memsize"=>672,
        "flags"=>{"wb_protected"=>true}
      })
    end

    it 'can dump complex objects' do
      file = File.new(__FILE__)
      info, klass, refs = dump_block do
        class Container
          def initialize
            @foo = []
            @bar = "yay"
          end

          def add(x)
            @foo << x
          end
        end

        c = Container.new
        c.add 1
        c.add "abc"
        c.add file
        c
      end

      expect(klass).to eq Container
      expect(refs).to eq [[1, "abc", file], "yay"]
      expect(info).to eq({
        "type"=>"OBJECT",
        "ivars"=>3,
        "flags"=>{"wb_protected"=>true}
      })
    end
  end

  describe '.info_for_obj' do
    it 'references the class from the instance' do
      class SampleClass1
      end
      info = subject.info_for_obj(SampleClass1.new)
      klass = addrs2objs([info['class']]).first
      expect(klass).to eq SampleClass1
    end

    it 'can dump class variables' do
      class SampleClass2
        def initialize
          @@bar = "yay"
        end
      end
      SampleClass2.new

      info = subject.info_for_obj(SampleClass2)
      refs = addrs2objs info['references']

      expect(refs).to include "yay"
    end

    it 'can dump trace information' do
      obj = nil
      ObjectSpace::trace_object_allocations do
        obj = "foobar"
      end

      info = subject.info_for_obj(obj)
      info.delete('class')
      expect(info).to eq({
        "type"=>"STRING",
        "embedded"=>true,
        "bytesize"=>6,
        "value"=>"foobar",
        "encoding"=>"UTF-8",
        "references"=>[],
        "file"=>"/Users/lfittl/Code/objspace-helpers/spec/lib/helpers_spec.rb",
        "line"=>__LINE__ - 13,
        "generation"=>GC.count,
        "flags"=>{"wb_protected"=>true}
      })
    end
  end
end
