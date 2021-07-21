<?php

/**
 * Page callback
 */
function booker_feedback_page() {
  drupal_add_js(
    drupal_get_path(
      'module',
      'customer_bonus_account'
    ) . '/js/booker_feedback_page.js'
  );
  drupal_add_css(
    drupal_get_path(
      'module',
      'customer_bonus_account'
    ) . '/css/booker_feedback_page.css'
  );

  $output = '';

  $block = module_invoke('webform', 'block_view', 'webform__client-block-6');
  $output .= render($block['content']);

  return $output;
}
