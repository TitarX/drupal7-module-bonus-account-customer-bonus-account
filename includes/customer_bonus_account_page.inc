<?php

/**
 * Page callback
 */
function customer_bonus_account_page() {
  drupal_add_js(
    drupal_get_path(
      'module',
      'customer_bonus_account'
    ) . '/js/customer_bonus_account.js'
  );
  drupal_add_css(
    drupal_get_path(
      'module',
      'customer_bonus_account'
    ) . '/css/customer_bonus_account.css'
  );

  $output = '';

  global $user;

  $current_user_bonus_product = commerce_product_load_by_sku($user->name);

  $number_bunus = $current_user_bonus_product->commerce_price['und'][0]['amount'];

  $output .= '<div id="number-bonus-wrapper">';
  $output .= 'У вас на счёте ';
  $output .= '<span id="number-bonus">';
  $output .= $number_bunus;
  $output .= '</span>';
  $output .= ' бонусов.';
  $output .= '</div>';

  $bonus_history_data = [];
  foreach ($current_user_bonus_product->field_bonus_history['und'] as $bonus_history_value) {
    $bonus_history_value_string = trim($bonus_history_value['value']);

    if (!empty($bonus_history_value_string) && $bonus_history_value_string != '0') {
      $bonus_history_value_array = explode('-', $bonus_history_value_string);

      if (count($bonus_history_value_array) == 2) {
        array_push($bonus_history_data, $bonus_history_value_array);
      }
    }
  }

  //Сортировка (Пузырьком) -
  $bonus_history_data_count = count($bonus_history_data);
  for ($i = 0; $i < $bonus_history_data_count - 1; $i++) {
    for ($j = $i + 1; $j < $bonus_history_data_count; $j++) {
      if ($bonus_history_data[$i][0] < $bonus_history_data[$j][0]) {
        $temp = $bonus_history_data[$i];
        $bonus_history_data[$i] = $bonus_history_data[$j];
        $bonus_history_data[$j] = $temp;
      }
    }
  }
  // - Сортировка (Пузырьком)

  $output .= '<h2 id="history-bonus-header">История зачисления бонусов</h2>';

  $output .= '<div id="history-bonus-wrapper">';
  $output .= '<table id="history-bonus-table">';
  $output .= '<tr class="table-header">';
  $output .= '<th>';
  $output .= 'Дата зачисления';
  $output .= '</th>';
  $output .= '<th>';
  $output .= 'Сумма';
  $output .= '</th>';
  $output .= '</tr>';
  foreach ($bonus_history_data as $bonus_history_data_key => $bonus_history_data_value) {
    $output .= '<tr class="row-' . $bonus_history_data_key . ' row-' . ($bonus_history_data_key % 2 == 0 ? 'even' : 'odd') . '">';
    $output .= '<td>';
    $output .= date('d.m.Y / H:i:s', $bonus_history_data_value[0]);
    $output .= '</td>';
    $output .= '<td>';
    $output .= $bonus_history_data_value[1];
    $output .= '</td>';
    $output .= '</tr>';
  }
  $output .= '</table>';
  $output .= '</div>';

  return $output;
}
