
require 'rubygems'
require 'ffi'

module HighGui
  
  CV_WINDOW_AUTOSIZE = 1
  
  extend FFI::Library
  ffi_lib 'highgui'
  attach_function :cvNamedWindow, :cvNamedWindow,[  :string, :uint ], :int
  
end


HighGui.cvNamedWindow( "test", 1);