require 'rake/clean'

cxx       = ENV['CXX']
boost     = ENV['BOOST_INCLUDE_PATH']
ccbase    = ENV['CCBASE_INCLUDE_PATH']
langflags = "-std=c++1y"
wflags    = "-Wall -Wextra -pedantic -Wno-unused-function -Wno-return-type-c-linkage"
archflags = "-march=native"
incflags  = "-I include -isystem #{boost} -isystem #{ccbase} -I ~/scratch/numa"
ldflags   = ""

if cxx.include? "clang"
	#optflags = "-Ofast -fno-fast-math -flto -DNEO_NO_DEBUG"
	optflags = "-O1 -ggdb"
elsif cxx.include? "g++"
	#optflags = "-Ofast -fno-fast-math -flto -fwhole-program"
	optflags = "-O1 -ggdb"
end

source_dir = "test"
test_sources = FileList["test/*.cpp"]
reference_dir = ""
reference_sources = ""

if RUBY_PLATFORM.include? "linux"
	ldflags = "-L/usa/aramesh/scratch/numa -pthread -lnuma -static-libstdc++"
	reference_dir = "reference/linux";
	reference_sources = FileList["reference/linux/*.cpp"]
elsif RUBY_PLATFORM.include? "darwin"
	reference_dir = "reference/os_x";
	reference_sources = FileList["reference/os_x/*.cpp"]
end

cxxflags = "#{langflags} #{wflags} #{archflags} #{incflags} #{optflags}"
dirs = ["data", "out"]
tests = test_sources.map{|f| f.sub(source_dir, "out").ext("run")}
refs = reference_sources.map{|f| f.sub(reference_dir, "out").ext("run")}

multitask :default => dirs + tests + refs

dirs.each do |d|
	directory d
end

tests.each do |f|
	src = f.sub("out", source_dir).ext("cpp")
	file f => [src] + dirs do
		sh "#{cxx} #{cxxflags} -o #{f} #{src} #{ldflags}"
	end
end

refs.each do |f|
	src = f.sub("out", reference_dir).ext("cpp")
	file f => [src] + dirs do
		sh "#{cxx} #{cxxflags} -o #{f} #{src} #{ldflags}"
	end
end

task :clobber do
	FileList["out/*.run"].each{|f| File.delete(f)}
end
