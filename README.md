# Universal role that installs apache httpd and configures it. (for CentOs6/7)

Note: Apache is configured to run in prefork mpm mode to support PHP.

Supported vars:
 - httpd_port (default: 80)
 - httpd_interface_name (default: false) - if false then listen on all
 - httpd_start_servers (default: 8)
 - httpd_min_spare_servers (default: 5)
 - httpd_max_spare_servers (default: 20)
 - httpd_server_limit (default: 256)
 - httpd_max_clients (default: 256)
 - httpd_max_requests_per_child (default: 4000)
 - httpd_vhost_root_directory (default: /var/www) - where vhost dirs are created
 - httpd_vhosts (default: []) - see vhosts.yml
 - https_allowed_ips (default: [])
 - httpd_allowed_group (default: na)

## Creates apache httpd vhosts and corresponding users which will be able to log in and access application files.

Supported vars:
 - httpd_vhosts (default: []) - each item shall contain:
   - name - used for file/dir naming
   - hostname
   - hostname_aliases (default: [])
   - admin_email (default: 'admin@' ~ hostname)
   - create_document_root (default: false)
   - document_root
   - log_directory
   - additional_config - additional config to append to vhost
   - additional_directory_config - additional config to append to vhost
   - priority (default: false) - prepended to host file name
