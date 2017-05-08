#!/bin/sh
ndvm_uuid=$(xec-vm -n Network get uuid)
inner_uuid=$(xec-vm -n innervm get uuid)
outer_uuid=$(xec-vm -n outervm get uuid)
sync_uuid=$(xec-vm -n syncvm get uuid)
uivm_uuid=$(xec-vm -n uivm get uuid)

xenstore-write /vm/${uivm_uuid}/linkwatch ""

# NDVM is not watching but still puts it's link state
xenstore-write /vm/${ndvm_uuid}/linkwatch ""
xenstore-write /vm/${ndvm_uuid}/link 1
xenstore-chmod /vm/${ndvm_uuid}/link b

# Outervm watches NDVM link state
xenstore-write /vm/${outer_uuid}/linkwatch ${ndvm_uuid} 
xenstore-write /vm/${outer_uuid}/link 1
xenstore-chmod /vm/${outer_uuid}/link b

xenstore-write /vm/${inner_uuid}/linkwatch ${outer_uuid} 
xenstore-write /vm/${inner_uuid}/link 1
xenstore-chmod /vm/${inner_uuid}/link b

xenstore-write /vm/${sync_uuid}/linkwatch ${inner_uuid}
xenstore-write /vm/${sync_uuid}/link 1
xenstore-chmod /vm/${sync_uuid}/link b
