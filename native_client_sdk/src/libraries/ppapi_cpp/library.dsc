{
  'TOOLS': ['newlib', 'glibc', 'pnacl', 'win', 'linux'],
  'SEARCH': [
      '../../../../ppapi/cpp',
      '../../../../ppapi/cpp/dev',
      '../../../../ppapi/cpp/extensions',
      '../../../../ppapi/cpp/extensions/dev',
      '../../../../ppapi/utility',
      '../../../../ppapi/utility/graphics',
      '../../../../ppapi/utility/threading',
      '../../../../ppapi/utility/websocket',
  ],
  'TARGETS': [
    {
      'NAME' : 'ppapi_cpp',
      'TYPE' : 'lib',
      'SOURCES' : [
        # ppapi/cpp
        'array_output.cc',
        'audio.cc',
        'audio_config.cc',
        'core.cc',
        'directory_entry.cc',
        'file_io.cc',
        'file_ref.cc',
        'file_system.cc',
        'fullscreen.cc',
        'graphics_2d.cc',
        'graphics_3d.cc',
        'graphics_3d_client.cc',
        'host_resolver.cc',
        'image_data.cc',
        'input_event.cc',
        'instance.cc',
        'instance_handle.cc',
        'media_stream_video_track.cc',
        'message_loop.cc',
        'module.cc',
        'mouse_cursor.cc',
        'mouse_lock.cc',
        'net_address.cc',
        'network_list.cc',
        'network_monitor.cc',
        'network_proxy.cc',
        'ppp_entrypoints.cc',
        'rect.cc',
        'resource.cc',
        'tcp_socket.cc',
        'text_input_controller.cc',
        'udp_socket.cc',
        'url_loader.cc',
        'url_request_info.cc',
        'url_response_info.cc',
        'var_array_buffer.cc',
        'var_array.cc',
        'var.cc',
        'var_dictionary.cc',
        'video_frame.cc',
        'view.cc',
        'websocket.cc',

        # ppapi/cpp/dev
        'alarms_dev.cc',
        'widget_client_dev.cc',
        'resource_array_dev.cc',
        'video_capture_client_dev.cc',
        'video_decoder_client_dev.cc',
        'crypto_dev.cc',
        'selection_dev.cc',
        'buffer_dev.cc',
        'url_util_dev.cc',
        'video_capture_dev.cc',
        'zoom_dev.cc',
        'ime_input_event_dev.cc',
        'text_input_dev.cc',
        'font_dev.cc',
        'find_dev.cc',
        'view_dev.cc',
        'memory_dev.cc',
        'truetype_font_dev.cc',
        'file_chooser_dev.cc',
        'video_decoder_dev.cc',
        'cursor_control_dev.cc',
        'device_ref_dev.cc',
        'printing_dev.cc',
        'scriptable_object_deprecated.cc',
        'audio_input_dev.cc',
        'scrollbar_dev.cc',
        'string_wrapper_dev.cc',
        'graphics_2d_dev.cc',
        'widget_dev.cc',
        'var_resource_dev.cc',

        # ppapi/cpp/extensions/dev
        'socket_dev.cc',

        # ppapi/utility/graphics
        'paint_aggregator.cc',
        'paint_manager.cc',

        # ppapi/utility/websocket
        'websocket_api.cc',

        # ppapi/utility/threading
        'lock.cc',
        'simple_thread.cc',
      ],
    }
  ],
  'HEADERS': [
    {
      'FILES': [
        'array_output.h',
        'audio_config.h',
        'audio.h',
        'completion_callback.h',
        'core.h',
        'directory_entry.h',
        'file_io.h',
        'file_ref.h',
        'file_system.h',
        'fullscreen.h',
        'graphics_2d.h',
        'graphics_3d_client.h',
        'graphics_3d.h',
        'host_resolver.h',
        'image_data.h',
        'input_event.h',
        'instance.h',
        'instance_handle.h',
        'logging.h',
        'media_stream_video_track.h',
        'message_loop.h',
        'module_embedder.h',
        'module.h',
        'module_impl.h',
        'mouse_cursor.h',
        'mouse_lock.h',
        'net_address.h',
        'network_list.h',
        'network_monitor.h',
        'network_proxy.h',
        'output_traits.h',
        'pass_ref.h',
        'point.h',
        'rect.h',
        'resource.h',
        'size.h',
        'tcp_socket.h',
        'text_input_controller.h',
        'touch_point.h',
        'udp_socket.h',
        'url_loader.h',
        'url_request_info.h',
        'url_response_info.h',
        'var_array_buffer.h',
        'var_array.h',
        'var_dictionary.h',
        'var.h',
        'video_frame.h',
        'view.h',
        'websocket.h',
      ],
      'DEST': 'include/ppapi/cpp',
    },
    {
      'FILES': [
        'alarms_dev.h',
        'array_dev.h',
        'audio_input_dev.h',
        'buffer_dev.h',
        'crypto_dev.h',
        'cursor_control_dev.h',
        'device_ref_dev.h',
        'file_chooser_dev.h',
        'find_dev.h',
        'font_dev.h',
        'graphics_2d_dev.h',
        'ime_input_event_dev.h',
        'may_own_ptr_dev.h',
        'memory_dev.h',
        'optional_dev.h',
        'printing_dev.h',
        'resource_array_dev.h',
        'scriptable_object_deprecated.h',
        'scrollbar_dev.h',
        'selection_dev.h',
        'string_wrapper_dev.h',
        'struct_wrapper_output_traits_dev.h',
        'text_input_dev.h',
        'to_c_type_converter_dev.h',
        'truetype_font_dev.h',
        'url_util_dev.h',
        'var_resource_dev.h',
        'video_capture_client_dev.h',
        'video_capture_dev.h',
        'video_decoder_client_dev.h',
        'video_decoder_dev.h',
        'view_dev.h',
        'widget_client_dev.h',
        'widget_dev.h',
        'zoom_dev.h',
      ],
      'DEST': 'include/ppapi/cpp/dev',
    },
    {
      'FILES': [
        'dict_field.h',
        'ext_output_traits.h',
        'from_var_converter.h',
        'optional.h',
        'to_var_converter.h',
      ],
      'DEST': 'include/ppapi/cpp/extensions',
    },
    {
      'FILES': [
        'socket_dev.h',
      ],
      'DEST': 'include/ppapi/cpp/extensions/dev',
    },
    {
      'FILES': [
        'completion_callback_factory.h',
        'completion_callback_factory_thread_traits.h',
      ],
      'DEST': 'include/ppapi/utility',
    },
    {
      'FILES': [
        'paint_aggregator.h',
        'paint_manager.h',
      ],
      'DEST': 'include/ppapi/utility/graphics',
    },
    {
      'FILES': [
        'paint_aggregator.h',
        'paint_manager.h',
      ],
      'DEST': 'include/ppapi/utility/graphics',
    },
    {
      'FILES': [
        'websocket_api.h',
      ],
      'DEST': 'include/ppapi/utility/websocket',
    },
    {
      'FILES': [
        'lock.h',
        'simple_thread.h',
      ],
      'DEST': 'include/ppapi/utility/threading',
    },
  ],
  'DEST': 'src',
  'NAME': 'ppapi_cpp',
}

