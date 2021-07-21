<?php

/**
 * Page callback
 */
function customer_bonus_account_withdrawal_page() {
  drupal_add_js(
    drupal_get_path(
      'module',
      'customer_bonus_account'
    ) . '/js/customer_bonus_account_withdrawal.js'
  );
  drupal_add_css(
    drupal_get_path(
      'module',
      'customer_bonus_account'
    ) . '/css/customer_bonus_account_withdrawal.css'
  );

  $output = '';

  $output .= '<div id="bonus_withdrawal_form_wrapper">';
  $customer_bonus_account_withdrawal_form = drupal_get_form(
    'customer_bonus_account_withdrawal_form'
  );
  $output .= drupal_render($customer_bonus_account_withdrawal_form);
  $output .= '</div>';

  $output .= '<div id="bonus_withdrawal_history_table_wrapper">';
  $output .= views_embed_view(
    'view_withdrawal_bonuses_orders',
    'block',
    $GLOBALS['user']->uid
  );
  $output .= '</div>';

  return $output;
}

function customer_bonus_account_withdrawal_form($form, &$form_state) {
  $form = [
    '#method' => 'post',
  ];

  $form['fields-wrapper'] = [
    '#type' => 'fieldset',
    '#title' => t('Создание заявки на вывод бонусов'),
    '#attributes' => ['class' => ['fields-wrapper']],
  ];

  $form['fields-wrapper']['inline-fields-wrapper'] = [
    '#type' => 'fieldset',
    '#attributes' => ['class' => ['inline-fields-wrapper']],
  ];

  $form['fields-wrapper']['inline-fields-wrapper']['bonus-amount'] = [
    '#type' => 'textfield',
    '#title' => t('Количество бонусов'),
    '#description' => '',
    '#attributes' => [
      'placeholder' => t(
        'Введите количество бонусов для снятия'
      ),
    ],
    '#maxlength' => 5,
    '#required' => TRUE,
  ];

  $payment_systems_select_options = ['0' => t('Выберите платёжную систему')];
  $payment_systems_names = get_payment_systems_names();
  foreach ($payment_systems_names as $payment_systems_names_key => $payment_systems_names_value) {
    $payment_systems_select_options[$payment_systems_names_key] = $payment_systems_names_value;
  }

  $form['fields-wrapper']['inline-fields-wrapper']['payment-systems-select'] = [
    '#title' => t('Платёжная система'),
    '#description' => '',
    '#type' => 'select',
    '#options' => $payment_systems_select_options,
    '#default_value' => '0',
    '#required' => TRUE,
  ];

  $form['fields-wrapper']['inline-fields-wrapper']['payment-system-account'] = [
    '#type' => 'textfield',
    '#title' => t('Счёт'),
    '#description' => '',
    '#attributes' => ['placeholder' => t('Укажите ваш номер счёта')],
    '#required' => TRUE,
  ];

  $form['fields-wrapper']['submit'] = [
    '#type' => 'submit',
    '#value' => t('Создать заявку'),
  ];

  return $form;
}

function customer_bonus_account_withdrawal_form_validate($form, &$form_state) {
  $bonus_amount = trim($form_state['values']['bonus-amount']);
  $user_product = commerce_product_load_by_sku($GLOBALS['user']->name);
  $total_amount_bonus = $user_product->commerce_price['und'][0]['amount'];
  if (preg_match(
      '/^[0-9]{1,5}$/uis',
      $bonus_amount
    ) !== 1 || $bonus_amount <= 0) {
    form_set_error(
      'bonus-amount',
      t(
        'Поле "Количество бонусов" заполнено некорректно, введите числовое значение больше нуля, до 5 знаков'
      )
    );
  }
  elseif ($bonus_amount > $total_amount_bonus) {
    form_set_error(
      'bonus-amount',
      t(
        'Сумма бонусов для выплаты превышает сумму бонусов на счёте'
      )
    );
  }

  $payment_systems_select = trim(
    $form_state['values']['payment-systems-select']
  );
  if (empty($payment_systems_select)) {
    form_set_error(
      'payment-systems-select',
      t(
        'Выберите платёжную систему, в которой хотите получить превод'
      )
    );
  }

  $payment_system_account = trim(
    $form_state['values']['payment-system-account']
  );
  if (preg_match('/[\'"<?>]/uis', $payment_system_account) === 1) {
    form_set_error(
      'payment-system-account',
      t('Поле "Счёт" содержит недопустимые символы')
    );
  }
}

function customer_bonus_account_withdrawal_form_submit($form, &$form_state) {
  $query = db_select('commerce_order');
  $query->fields('commerce_order', ['order_id']);
  $db_and = db_and();
  $db_and->condition('uid', $GLOBALS['user']->uid, '=');
  $db_and->condition('status', 'pending', '=');
  $query->condition($db_and);
  $user_order_pending_status = $query->execute()->fetchAll();

  if (empty($user_order_pending_status)) {
    $order = commerce_order_new($GLOBALS['user']->uid);
    commerce_order_save($order);

    $user_product = commerce_product_load_by_sku($GLOBALS['user']->name);
    $line_item = commerce_product_line_item_new(
      $user_product,
      1,
      $order->order_id
    );
    commerce_line_item_save($line_item);

    $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
    $order_wrapper->commerce_line_items[] = $line_item;

    $order->field_amount_bonus_for_order['und'][0]['value'] = trim(
      $form_state['values']['bonus-amount']
    );

    $payment_systems_select = trim(
      $form_state['values']['payment-systems-select']
    );
    $payment_system_name = get_payment_system_name($payment_systems_select);
    $order->field_payment_system_name['und'][0]['value'] = trim(
      $payment_system_name
    );

    $order->field_payment_account_number['und'][0]['value'] = trim(
      $form_state['values']['payment-system-account']
    );

    $order->field_field_total_amount_bonus['und'][0]['value'] = $user_product->commerce_price['und'][0]['amount'];

    $order->field_bonus_order_user_name['und'][0]['value'] = $GLOBALS['user']->name;

    $order->status = 'pending';
    commerce_order_save($order);

    $commands = [];
    $commands[] = ['command' => 'reloadPage'];
    $commands[] = [
      'message' => drupal_set_message(
        'Заявка на вывод бонусов успешно создана',
        'status',
        FALSE
      ),
    ];
    return ['#type' => 'ajax', '#commands' => $commands];
  }
  else {
    $commands = [];
    $commands[] = ['command' => 'reloadPage'];
    $commands[] = [
      'message' => drupal_set_message(
        'Заявка на вывод бонусов не создана, у вас уже имеется активная заявка',
        'warning',
        FALSE
      ),
    ];
    return ['#type' => 'ajax', '#commands' => $commands];
  }
}
