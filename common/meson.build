common_sources = files(
  'actions.vala',
  'add-keyserver-dialog.vala',
  'app-settings.vala',
  'backend.vala',
  'catalog.vala',
  'deletable.vala',
  'delete-dialog.vala',
  'deleter.vala',
  'exportable.vala',
  'exporter.vala',
  'icons.vala',
  'interaction.vala',
  'item-list.vala',
  'lockable.vala',
  'object.vala',
  'passphrase-prompt.vala',
  'pgp-settings.vala',
  'place.vala',
  'predicate.vala',
  'prefs.vala',
  'registry.vala',
  'servers.vala',
  'types.vala',
  'util.vala',
  'validity.vala',
  'viewable.vala',
)

common_deps = [
  glib_deps,
  gtk,
  gcr,
  gcr_ui,
  config,
]

common_vala_args = [
]

if get_option('keyservers-support')
  common_sources += 'keyserver-control.vala'
  common_vala_args += [ '-D', 'WITH_KEYSERVER' ]
endif

common_lib = static_library('common',
  common_sources,
  dependencies: common_deps,
  vala_args: common_vala_args,
  vala_header: 'seahorse-common.h',
)

common_dep = declare_dependency(
  link_with: common_lib,
  include_directories: include_directories('.'),
  dependencies: common_deps,
)
