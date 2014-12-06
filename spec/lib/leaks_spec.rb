require 'spec_helper'

describe ObjspaceHelpers do
  subject { described_class }

  describe '.find_leaks' do

    it 'detects class variables as leaks' do
      class SampleClass
        def initialize
          @@my_class_variable = "yay"
        end
      end

      leaks = subject.find_leaks do
        SampleClass.new
      end

      expect(leaks.size).to be 1
      expect(leaks[0].klass.dereference).to eq String
      expect(leaks[0].info["value"]).to eq "yay"
      expect(leaks[0].referenced_by.size).to be 1
      expect(leaks[0].referenced_by[0].dereference).to eq SampleClass
    end

    it 'does not detect instance variables as leaks' do
      class SampleClass2
        def initialize
          @my_instance_variable = "yay"
        end
      end

      leaks = subject.find_leaks do
        SampleClass2.new
      end

      expect(leaks).to be_empty
    end

    it 'detects symbols as leaks' do
      leaks = subject.find_leaks do
        'sample_symbol_2'.to_sym
      end

      expect(leaks.size).to be 1
      expect(leaks[0].klass.dereference).to eq String
      expect(leaks[0].info['value']).to eq 'sample_symbol_2'
      expect(leaks[0].info['frozen']).to be true
      expect(leaks[0].info['fstring']).to be true
    end

    it 'does not detect strings as leaks' do
      leaks = subject.find_leaks do
        'sample_symbol_2'
      end

      expect(leaks).to be_empty
    end
  end

  describe '.find_leak_sources' do
    it 'works' do
      class SampleClass3
        def initialize
          @@my_class_variable = "yay"
        end
      end

      top_level_leaks, leaks_by_source = subject.find_leak_sources do
        'sample_symbol_3'.to_sym

        SampleClass3.new
      end

      expect(leaks_by_source.keys.size).to eq 1
      expect(leaks_by_source.keys[0].info['type']).to eq 'CLASS'
      expect(leaks_by_source.keys[0].dereference).to eq SampleClass3

      expect(top_level_leaks.size).to eq 1
      expect(top_level_leaks[0].info['type']).to eq 'STRING'
      expect(top_level_leaks[0].info['value']).to eq 'sample_symbol_3'
    end

    it 'contains file information' do
      class SampleClass4
        def initialize
          @@my_class_variable = "yay"
        end
      end

      top_level_leaks, leaks_by_source = subject.find_leak_sources(trace: true) do
        SampleClass4.new
      end

      expect(leaks_by_source.keys.size).to eq 1
      expect(leaks_by_source.keys[0].info['type']).to eq 'CLASS'
      expect(leaks_by_source.keys[0].dereference).to eq SampleClass4
      expect(leaks_by_source.values[0].size).to eq 1
      expect(leaks_by_source.values[0][0].info['file']).to eq __FILE__
      expect(leaks_by_source.values[0][0].info['line']).to eq __LINE__ - 13

      expect(top_level_leaks).to be_empty
    end
  end
end
