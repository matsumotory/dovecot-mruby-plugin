MRuby::Build.new do |conf|
  if ENV['VisualStudioVersion'] || ENV['VSINSTALLDIR']
    toolchain :visualcpp
  else
    toolchain :gcc
  end

  enable_debug

  conf.gembox 'full-core'

  conf.cc do |cc|
    cc.flags = [ENV['CFLAGS'], '-fPIC']
  end
end
